#ifndef APPLICATION_HANDLER_HPP
#define APPLICATION_HANDLER_HPP

#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <algorithm>

#include "agora/utils/bitmask.hpp"

#include "agora/fs_handler.hpp"
#include "agora/launcher.hpp"
#include "agora/logger.hpp"
#include "agora/remote_handler.hpp"

namespace agora {

using client_id_t = std::string;
using configuration_id_t = std::string;
using client_list_t = std::unordered_set<client_id_t>;

class RemoteApplicationHandler {
public:
  enum class ApplicationStatus : uint_fast16_t {
    RECOVERING = (1u << 0),
    CLUELESS = (1u << 1),
    UNDEFINED = (1u << 2),
    WITH_INFORMATION = (1u << 3),
    EXPLORING = (1u << 4),
    BUILDING_DOE = (1u << 5),
    WITH_DOE = (1u << 6),
    BUILDING_CLUSTER = (1u << 7),
    WITH_CLUSTER = (1u << 8),
    BUILDING_MODEL = (1u << 9),
    WITH_MODEL = (1u << 10),
    BUILDING_PREDICTION = (1u << 11),
    WITH_PREDICTION = (1u << 12),

    _bitmask_max_element = WITH_PREDICTION
  };

  RemoteApplicationHandler(const application_id &application_id, const FsConfiguration &fs_config,
                           const LauncherConfiguration &launcher_config);
  ~RemoteApplicationHandler();

  void welcome_client(const client_id_t &cid, const std::string &info);

  void bye_client(const client_id_t &cid);

  void process_observation(const client_id_t &cid, long duration_sec, long duration_ns, const std::string &observation_values);

private:
  const application_id app_id;
  const std::string LOG_HEADER;

  std::mutex app_mutex;
  bitmask::bitmask<ApplicationStatus> status;

  int iteration_number;
  int num_configurations_per_iteration;
  int num_configurations_sent_per_iteration;

  client_list_t active_clients;

  margot::heel::block_model description;
  doe_model doe;
  cluster_model cluster;
  prediction_model prediction;

  std::shared_ptr<FsHandler> fs_handler;
  std::shared_ptr<Logger> logger;
  std::shared_ptr<RemoteHandler> remote;

  // launchers
  LauncherConfiguration launcher_configuration;
  std::shared_ptr<Launcher> doe_launcher;
  std::unordered_map<std::string, std::shared_ptr<Launcher>> model_launchers;
  std::shared_ptr<Launcher> cluster_launcher;
  std::shared_ptr<Launcher> prediction_launcher;

  // utility functions
  bool parse_informations(const std::string& info);
  bool parse_observation(const std::string& observation_values, margot::heel::block_model& op);
  void initialize_plugin_launchers();
  const std::string configuration_to_json(const configuration_model &configuration) const;
  const std::string prediction_to_json(const prediction_model &prediction) const;

  // check functions
  bool are_features_enabled() const
  {
    return !description.features.fields.empty();
  }
  bool is_doe_valid() const
  {
    return !doe.required_explorations.empty();
  }
  bool is_cluster_valid() const
  {
    return !cluster.centroids.empty();
  }
  bool is_prediction_valid() const
  {
    return !prediction.predicted_results.empty();
  }
  bool are_models_valid() const
  {
    auto metric_itr = std::find_if_not(description.metrics.begin(), description.metrics.end(),
                                       [&](const auto &metric) { return fs_handler->is_model_valid(app_id, metric.name); });
    return (metric_itr == description.metrics.end());
  }

  // building functions
  pid_t start_doe();
  std::vector<pid_t> start_modeling();
  pid_t start_clustering();
  pid_t start_prediction();

  // recovery functions
  void start_recovering();

  // message sending functions
  void send_abort_message(const client_id_t &cid) {
    remote->send_message({MESSAGE_HEADER + "/" + app_id.str() + "/" + cid + "/abort", ""});
  }
  void send_configuration(const client_id_t &cid)
  {
    if (num_configurations_sent_per_iteration <= num_configurations_per_iteration)
    {
      auto configuration = doe.get_next();
      if (configuration != doe.required_explorations.end())
      {
        remote->send_message({MESSAGE_HEADER + "/" + app_id.str() + "/" + cid + "/explore", configuration_to_json(configuration->second)});

        doe.update_config(configuration->first);

        num_configurations_sent_per_iteration++;
      }
    }
  }
  void send_prediction(const client_id_t &cid) const
  {
    remote->send_message({MESSAGE_HEADER + "/" + app_id.str() + "/" + cid + "/prediction", prediction_to_json(prediction)});
  }
  void broadcast_prediction() const
  {
    remote->send_message({MESSAGE_HEADER + "/" + app_id.str() + "/prediction", prediction_to_json(prediction)});
  }
};

BITMASK_DEFINE(RemoteApplicationHandler::ApplicationStatus)

} // namespace agora

#endif // APPLICATION_HANDLER_HPP
