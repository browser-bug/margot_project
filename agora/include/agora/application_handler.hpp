/* Agora library
 * Copyright (C) 2021 Bernardo Menicagli
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef APPLICATION_HANDLER_HPP
#define APPLICATION_HANDLER_HPP

#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <algorithm>
#include <atomic>

#include "agora/model_doe.hpp"
#include "agora/utils/bitmask.hpp"

#include "agora/fs_handler.hpp"
#include "agora/launcher.hpp"
#include "agora/logger.hpp"
#include "agora/remote_handler.hpp"

namespace agora {

/**
 * @brief Unique identifier of an active client (CID).
 */
using client_id_t = std::string;
/**
 * @brief A list of CIDs.
 */
using client_list_t = std::unordered_set<client_id_t>;

/**
 * @brief The implementation of a generic Remote Application Handler (RAH).
 *
 * @details
 * This class represents the interface of the Agora online module toward the application. Its main role is to coordinate the online learning
 * process launching the remote plugins. The internal logic is based on a shared internal state which is changed depending on the phase that
 * is being run. In particular, the main phases can be summarized as follows:
 *  -# A new client connects with a welcome message and it's added to the pool of available clients.
 *  -# The DOE plugin is launched generating new configurations to explore.
 *  -# The DSE phase starts, exploring the configurations generated.
 *  -# A series of observations are processed as soon as they are received by the remote handler.
 *  -# After reaching a certain number of observations, the modelling and clustering phases start.
 *  -# If the models generated are deemed eligible, the final predictions are computed and the application knowledge is broadcasted.
 *
 * @note
 * The main methods are mutex protected in order to enforce a consistent internal state.
 */
class RemoteApplicationHandler {
public:
    /**
     * @brief The internal state which drive the online learning process.
     *
     * @details
     * This implements a state machine that keeps track of the progress the remote handler makes during the process.
     */
    enum class InternalStatus : uint_fast16_t {
        RECOVERING = (1u << 0),        ///< The remote handler is recovering from a crash, looking from previous data to load.
        CLUELESS = (1u << 1),          ///< The remote handler has just started or cannot infer a previous state.
        UNDEFINED = (1u << 2),         ///< The remote handler reached an undefined state and the process needs to be restarted.
        WITH_INFORMATION = (1u << 3),  ///< The remote handler parsed the application informations correctly.
        EXPLORING = (1u << 4),         ///< The DSE phase started and we're exploring the configurations produced.
        BUILDING_DOE = (1u << 5),      ///< The DOE plugin is building new experiments (configurations).
        WITH_DOE = (1u << 6),          ///< The DOE plugin correctly produced a list of configurations to explore.
        BUILDING_CLUSTER = (1u << 7),  ///< The Clustering plugin is finding new cluster inside the input features space.
        WITH_CLUSTER = (1u << 8),      ///<  The Clustering plugin correctly produced a list of cluster representatives (i.e. centroids)
        BUILDING_MODEL = (1u << 9),    ///< The Modelling plugin is training new models for each EFPs.
        WITH_MODEL = (1u << 10),       ///< The Modelling plugin correctly produced a model for each EFPs verifying the quality thresholds.
        BUILDING_PREDICTION = (1u << 11),  ///< The Predicting plugin is using the produced models to predict the final knowledge.
        WITH_PREDICTION = (1u << 12),      ///< The Predicting plugin correctly produced the final predictions.

        _bitmask_max_element = WITH_PREDICTION
    };

    /**
     * @brief Construct a new RAH instance.
     *
     * @param [in] app_id The AID corresponding to this remote handler.
     * @param [in] fs_config The configuration to use to get an instance of the storage handler.
     * @param [in] launcher_config The generic configuration to use to get an instance of the plugin launcher.
     *
     * @details
     * Create a new instance of a RAH setting the state to CLUELESS. Get the shared instances of the Logger and the Remote Message Handler.
     * Get a new instance of the Storage Handler and the Plugin Launcher based on the corresponding configurations.
     */
    RemoteApplicationHandler(const application_id &app_id, const FsConfiguration &fs_config, const LauncherConfiguration &launcher_config);
    /**
     * @brief Destruct the RAH instance.
     *
     * @details
     * Delete all the data relative to the AID in storage and clear the launcher workspaces.
     */
    ~RemoteApplicationHandler();

    /**
     * @brief Handle a welcome message from a new client.
     *
     * @param [in] cid The client id.
     * @param [in] info The application informations to be parsed corresponding to the message payload. Here we expect a description of the
     *  software-knobs, input-features and EFPs defined inside the mARGOt configuration file plus all the parameters needed during the
     *  online learning process.
     *
     * @details
     * Add the new client to the pool of active clients. Depending on the internal status, checks if:
     *  - It is the first client connecting, in which case the informations are parsed, storing the application description data and
     *    initializing the plugin launchers.
     *  - If we're already performing DSE, send a new configuration to the client.
     *  - If the application knowledge is available already, send it to the client.
     */
    void welcome_client(const client_id_t &cid, const std::string &info);

    /**
     * @brief Handle a bye message from an active client.
     *
     * @param [in] cid The client id.
     *
     * @details
     * Remove the client from the pool of active clients. If it was the last client connected, free up the memory by clearing any internal
     * data stored in RAM (i.e. DOE, clusters and predictions) and reset the status to CLUELESS.
     */
    void bye_client(const client_id_t &cid);

    /**
     * @brief Handle a new observation from an active client.
     *
     * @param [in] cid The client id.
     * @param [in] duration_sec The observation timestamp in seconds.
     * @param [in] duration_ns The observation timestamp in nanoseconds.
     * @param [in] observation_values A string corresponding the observed values formatted as an Operating Point to be parsed.
     *
     * @details
     * Perform the following steps in order:
     *  -# Check the internal state to make sure we're currently inside the DSE phase.
     *  -# Parse the observed values and insert a new entry into the observation table.
     *  -# Check if we have still configurations available and if we've not reached the max number of configuration per iteration, then
     *     sends a new configuration to the client.
     *  -# Otherwise, start the modelling and clustering process in parallel.
     *  -# If the models found are deemed eligible, starts the predicting process and create a new application knowledge to be broadcasted.
     */
    void process_observation(const client_id_t &cid, long duration_sec, long duration_ns, const std::string &observation_values);

private:
    /**
     * @brief The unique identifier of an application (AID).
     */
    const application_id app_id;
    /**
     * @brief The header to use during logs of the RAH.
     */
    const std::string LOG_HEADER;

    /**
     * @brief The mutex used to enforce a consistent internal state.
     */
    std::mutex app_mutex;
    /**
     * @brief The bitmask implementing the internal states of the RAH.
     */
    bitmask::bitmask<InternalStatus> handler_status;
    /**
     * @brief Set a new internal state.
     *
     * @param [in] state The new state to set.
     * @param [in] clear Whether to clear the previous state or to combine it with a bitwise OR.
     */
    void set_state(bitmask::bitmask<InternalStatus> state, bool clear = true) { handler_status = clear ? state : (handler_status | state); }
    /**
     * @brief Clear a specific internal state.
     *
     * @param [in] state The state to unset.
     */
    void unset_state(bitmask::bitmask<InternalStatus> state) { handler_status &= ~state; }
    /**
     * @brief Toggle a specific internal state.
     *
     * @param [in] state The state to toggle.
     */
    void toggle_state(bitmask::bitmask<InternalStatus> state) { handler_status ^= state; }
    /**
     * @brief Check if a specific internal state is on.
     *
     * @param [in] state The state to check.
     *
     * @returns True if the state is on, False otherwise.
     */
    bool check_state(bitmask::bitmask<InternalStatus> state) { return ((handler_status & state) == state); }

    /**
     * @brief The current iteration number.
     */
    int iteration_number;
    /**
     * @brief The maximum number of configurations to explore each iteration.
     */
    const int num_configurations_per_iteration;
    /**
     * @brief A counter for the number of configurations sent at the current iteration.
     */
    int num_configurations_sent_per_iteration;

    /**
     * @brief The list of active clients.
     */
    client_list_t active_clients;
    /**
     * @brief Add a new client to the pool of active clients.
     *
     * @param [in] cid The client id.
     */
    void add_client(const client_id_t &cid) { active_clients.insert(cid); }
    /**
     * @brief Remove the specified client from the pool of active clients.
     *
     * @param [in] cid The client id.
     */
    void remove_client(const client_id_t &cid) { active_clients.erase(cid); }

    /**
     * @brief A data structure representing the application description.
     */
    margot::heel::block_model description;
    /**
     * @brief A data structure representing a list of DOE configurations.
     */
    doe_model doe;
    /**
     * @brief A data structure representing the input features centroids.
     */
    cluster_model cluster;
    /**
     * @brief A data structure representing a list of predictions.
     */
    prediction_model prediction;
    /**
     * @brief Check if input features are enabled for the application.
     *
     * @returns True if the input features are available, False otherwise.
     */
    bool are_features_enabled() const { return !description.features.fields.empty(); }
    /**
     * @brief Check if a good model has been found for each EFP.
     *
     * @returns True if a model is available for each EFP, False otherwise.
     */
    bool are_models_valid() {
        auto metric_itr = std::find_if_not(description.metrics.begin(), description.metrics.end(),
                                           [&](const auto &metric) { return fs_handler->is_model_valid(app_id, metric.name); });
        return (metric_itr == description.metrics.end());
    }
    /**
     * @brief Check if at least one DOE configuration is available.
     *
     * @returns True if a list of DOE configurations are available, False otherwise.
     */
    bool is_doe_valid() { return !doe.required_explorations.empty(); }
    /**
     * @brief Check if at least one cluster representative has been found.
     *
     * @returns True if a list of cluster centroids are available, False otherwise.
     */
    bool is_cluster_valid() { return !cluster.centroids.empty(); }
    /**
     * @brief Check if a list of predictions is available.
     *
     * @returns True if the predictions are available, False otherwise.
     */
    bool is_prediction_valid() { return !prediction.predicted_results.empty(); }

    /**
     * @brief A pointer to the storage handler.
     */
    std::shared_ptr<FsHandler> fs_handler;
    /**
     * @brief A pointer to the logger.
     */
    std::shared_ptr<Logger> logger;
    /**
     * @brief A pointer to the remote message handler.
     */
    std::shared_ptr<RemoteHandler> remote;

    /**
     * @brief A generic plugin launcher configuration to be used during the creation of a new instance.
     */
    LauncherConfiguration launcher_configuration;
    /**
     * @brief Create a new instance for every plugin launcher inside the RAH and initialize their workspace.
     */
    void initialize_plugin_launchers();
    /**
     * @brief A pointer to the DOE plugin launcher.
     */
    std::shared_ptr<Launcher> doe_launcher;
    /**
     * @brief Start the DOE plugin launcher.
     *
     * @returns A process ID generated by forking the caller process.
     */
    pid_t start_doe();
    /**
     * @brief A pointer to the Clustering plugin launcher.
     */
    std::shared_ptr<Launcher> cluster_launcher;
    /**
     * @brief Start the Clustering plugin launcher.
     *
     * @returns A process ID generated by forking the caller process.
     */
    pid_t start_clustering();
    /**
     * @brief A pointer to the Predicting plugin launcher.
     */
    std::shared_ptr<Launcher> prediction_launcher;
    /**
     * @brief Start the Predicting plugin launcher.
     *
     * @returns A process ID generated by forking the caller process.
     */
    pid_t start_prediction();
    /**
     * @brief A map of Modelling plugin launchers for each application EFP.
     *
     * @details
     *  - Key: name of the EFP.
     *  - Value: a pointer to the corresponding launcher.
     */
    std::unordered_map<std::string, std::shared_ptr<Launcher>> model_launchers;
    /**
     * @brief Start the Modelling plugin launchers.
     *
     * @returns A vector of process IDs generated by forking the caller process for each EFP.
     */
    std::vector<pid_t> start_modeling();
    /**
     * @brief Clear the workspace of every plugin launcher.
     */
    void clear_launchers() {
        static auto clear = [](const auto &launcher) {
            if (launcher) {
                launcher->clear_workspace();
            }
        };
        clear(doe_launcher);
        clear(prediction_launcher);
        clear(cluster_launcher);
        clear(doe_launcher);
        for (const auto &launcher : model_launchers) {
            clear(launcher.second);
        }
    }

    /**
     * @brief Parse and extract the application informations.
     *
     * @param [in] info String corresponding to the application informations to be parsed.
     * @param [in, out] description A data structure representing the application description.
     *
     * @returns True if the operation was successful, False otherwise.
     */
    bool parse_informations(const std::string &info, margot::heel::block_model &description);
    /**
     * @brief Parse and extract a new observation.
     *
     * @param [in] observation_values String corresponding to the observation to be parsed.
     * @param [in, out] op A data structure representing the observation as an Operating Point.
     *
     * @returns True if the operation was successful, False otherwise.
     */
    bool parse_observation(const std::string &observation_values, margot::heel::block_model &op);
    /**
     * @brief Convert a configuration model to a JSON format.
     *
     * @param [in] configuration A data structure representing the configuration.
     *
     * @returns A string representing the configuration in JSON format.
     */
    std::string configuration_to_json(const configuration_t &configuration) const;
    /**
     * @brief Convert a final prediction model to a JSON format.
     *
     * @param [in] prediction A data structure representing the prediction.
     *
     * @returns A string representing the prediction in JSON format.
     *
     * @details
     * This method is used to generate the final application knowledge, creating a JSON string compatible with the mARGOt autotuner. That
     * means that it creates a list of Operating Points, leveraging the input features clusters and the predictions made by the best models
     * found.
     */
    std::string prediction_to_json(const prediction_model &prediction) const;

    /**
     * @brief Check the storage to find if any data is available and load it in memory.
     */
    void start_recovering();

    /**
     * @brief Send a new message to abort the process and notify the specified client.
     *
     * @param [in] cid The client id.
     */
    void send_abort_message(const client_id_t &cid) {
        remote->send_message({MESSAGE_HEADER + "/" + app_id.str() + "/" + cid + "/abort", ""});
    }
    /**
     * @brief Send a new configuration to the specified client.
     *
     * @param [in] cid The client id.
     *
     * @returns True if the message is correctly sent, False otherwise.
     *
     * @details
     * This method gets the next available configuration from the data structure storing the list of configurations, then sends it in JSON
     * format to the client.
     */
    bool send_configuration(const client_id_t &cid) {
        configuration_model doe_entry;
        if (doe.get_next(doe_entry)) {
            remote->send_message(
                    {MESSAGE_HEADER + "/" + app_id.str() + "/" + cid + "/explore", configuration_to_json(doe_entry.configuration)});

            num_configurations_sent_per_iteration++;
            return true;
        }
        return false;
    }
    /**
     * @brief Send the final predictions to the specified client.
     *
     * @param [in] cid The client id.
     */
    void send_prediction(const client_id_t &cid) const {
        remote->send_message({MESSAGE_HEADER + "/" + app_id.str() + "/" + cid + "/prediction", prediction_to_json(prediction)});
    }
    /**
     * @brief Broadcast the final predictions to all the active clients.
     */
    void broadcast_prediction() const {
        remote->send_message({MESSAGE_HEADER + "/" + app_id.str() + "/prediction", prediction_to_json(prediction)});
    }
};

BITMASK_DEFINE(RemoteApplicationHandler::InternalStatus)

}  // namespace agora

#endif // APPLICATION_HANDLER_HPP
