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

#ifndef FS_HANDLER_HPP
#define FS_HANDLER_HPP

#include <filesystem>
#include <memory>

#include "agora/fs_cluster.hpp"
#include "agora/fs_configuration.hpp"
#include "agora/fs_description.hpp"
#include "agora/fs_doe.hpp"
#include "agora/fs_observation.hpp"
#include "agora/fs_prediction.hpp"
#include "agora/fs_model.hpp"
#include "agora/plugin_configuration.hpp"

namespace agora {

/**
 * @brief Wrapper class which provides an API to interact with the storage.
 *
 * @details
 * This wrapper hides the storage handler implementations by providing an API containing the functions to interact with different sections
 * of the storage. It is also in charge of creating an environmental file that is used by a generic plugin to launch their script.
 */
class FsHandler {
public:
    /**
     * @brief Get a new instance of the FsHandler.
     *
     * @param [in] configuration The storage handler FsConfiguration to use.
     *
     * @returns A pointer to the filesystem handler instantiated.
     */
    static std::shared_ptr<FsHandler> get_instance(const FsConfiguration &configuration) {
        return std::shared_ptr<FsHandler>(new FsHandler(configuration));
    }

    /**
     * @brief Store the cluster data depending on the storage implementation.
     *
     * @tparam T The plugin type which will be using the generate environmental configuration.
     * @param [in, out] pc The data structure containing the plugin configuration data to use.
     *
     * @details
     * Depending on the plugin type T, this method creates different types of environmental configurations always starting from the
     * insertion of a preamble.
     *
     * @see env_configuration_preamble()
     */
    template <PluginType T>
    void create_env_configuration(PluginConfiguration &pc);

    /**
     * @brief Wrap the method to store the description data.
     *
     * @see FsDescription::store_description()
     */
    void store_description(const application_id &app_id, const margot::heel::block_model &description) {
        description_fs->store_description(app_id, description);
    }
    /**
     * @brief Wrap the method to load the description data.
     *
     * @see FsDescription::load_description()
     */
    margot::heel::block_model load_description(const application_id &app_id) { return description_fs->load_description(app_id); }

    /**
     * @brief Wrap the method to store the predictions data.
     *
     * @see FsPrediction::store_prediction()
     */
    void store_prediction(const application_id &app_id, const margot::heel::block_model &description, const prediction_model &model) {
        prediction_fs->store_prediction(app_id, description, model);
    }
    /**
     * @brief Wrap the method to load the predictions data.
     *
     * @see FsPrediction::load_prediction()
     */
    prediction_model load_prediction(const application_id &app_id, const margot::heel::block_model &description) {
        return prediction_fs->load_prediction(app_id, description);
    }

    /**
     * @brief Wrap the method to check the model validity.
     *
     * @see FsModel::is_model_valid()
     */
    bool is_model_valid(const application_id &app_id, const std::string &metric_name) const {
        return model_fs->is_model_valid(app_id, metric_name);
    }

    /**
     * @brief Wrap the method to store the DOE data.
     *
     * @see FsDoe::store_doe()
     */
    void store_doe(const application_id &app_id, const margot::heel::block_model &description, const doe_model &doe) {
        doe_fs->store_doe(app_id, description, doe);
    }
    /**
     * @brief Wrap the method to load the DOE data.
     *
     * @see FsDoe::load_doe()
     */
    doe_model load_doe(const application_id &app_id, const margot::heel::block_model &description) {
        return doe_fs->load_doe(app_id, description);
    }
    /**
     * @brief Wrap the method to update the specified DOE configuration.
     *
     * @see FsDoe::update_doe()
     */
    void update_doe(const application_id &app_id, const margot::heel::block_model &description, const std::string &config_id) {
        return doe_fs->update_doe(app_id, description, config_id);
    }
    /**
     * @brief Wrap the method to remove all the DOE configurations.
     *
     * @see FsDoe::empty_doe_entries()
     */
    void empty_doe_entries(const application_id &app_id, const margot::heel::block_model &description) {
        return doe_fs->empty_doe_entries(app_id, description);
    }

    /**
     * @brief Wrap the method to create the observations data container.
     *
     * @see FsObservation::create_observation_table()
     */
    void create_observation_table(const application_id &app_id, const margot::heel::block_model &description) {
        observation_fs->create_observation_table(app_id, description);
    }
    /**
     * @brief Wrap the method to insert a new observation.
     *
     * @see FsObservation::insert_observation_entry()
     */
    void insert_observation_entry(const application_id &application_id, const std::string &client_id, const long duration_sec,
                                  const long duration_ns, const margot::heel::operating_point_model &operating_point) {
        observation_fs->insert_observation_entry(application_id, client_id, duration_sec, duration_ns, operating_point);
    }

    /**
     * @brief Wrap the method to store the cluster data.
     *
     * @see FsCluster::store_cluster()
     */
    void store_cluster(const application_id &app_id, const margot::heel::block_model &description, const cluster_model &cluster) {
        cluster_fs->store_cluster(app_id, description, cluster);
    }
    /**
     * @brief Wrap the method to load the cluster data.
     *
     * @see FsCluster::load_cluster()
     */
    cluster_model load_cluster(const application_id &app_id, const margot::heel::block_model &description) {
        return cluster_fs->load_cluster(app_id, description);
    }

    /**
     * @brief Wrap the methods to delete each section of data inside their corresponding storage locations.
     *
     * @see FsDescription::erase(), FsDoe::erase(), FsPrediction::erase(), FsModel::erase(), FsObservation::erase(), FsCluster::erase()
     */
    void erase(const application_id &app_id) {
        description_fs->erase(app_id);
        doe_fs->erase(app_id);
        prediction_fs->erase(app_id);
        model_fs->erase(app_id);
        observation_fs->erase(app_id);
        cluster_fs->erase(app_id);
    }

private:
    /**
     * @brief Constructor which uses the configuration provided to get a new instance of each of the storage handlers.
     *
     * @param [in] configuration The storage handler FsConfiguration to use.
     */
    FsHandler(const FsConfiguration &configuration);

    /**
     * @brief Insert a preamble inside the environmental configuration containing the Agora properties location and the storage type for
     *  each section of data.
     *
     * @param [in, out] pc The data structure containing the plugin configuration data to use.
     */
    void env_configuration_preamble(PluginConfiguration &pc);

    /**
     * @brief A pointer to the global Logger.
     */
    std::shared_ptr<Logger> logger;

    /**
     * @brief A pointer to the FsDescription handler.
     */
    std::unique_ptr<FsDescription> description_fs;
    /**
     * @brief A pointer to the FsDoe handler.
     */
    std::unique_ptr<FsDoe> doe_fs;
    /**
     * @brief A pointer to the FsPrediction handler.
     */
    std::unique_ptr<FsPrediction> prediction_fs;
    /**
     * @brief A pointer to the FsModel handler.
     */
    std::unique_ptr<FsModel> model_fs;
    /**
     * @brief A pointer to the FsCluster handler.
     */
    std::unique_ptr<FsCluster> cluster_fs;
    /**
     * @brief A pointer to the FsObservation handler.
     */
    std::unique_ptr<FsObservation> observation_fs;
};

}  // namespace agora

#endif // FS_HANDLER_HPP
