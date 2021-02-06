#include "agora/fs_handler.hpp"
#include "agora/application_manager.hpp"

namespace agora {

FsHandler::FsHandler(const FsConfiguration &configuration)
{
  ApplicationManager &am = ApplicationManager::get_instance();
  logger = am.get_logger();

  description_fs = FsDescription::get_instance(configuration);
  doe_fs = FsDoe::get_instance(configuration);
  prediction_fs = FsPrediction::get_instance(configuration);
  model_fs = FsModel::get_instance(configuration);
  observation_fs = FsObservation::get_instance(configuration);
  cluster_fs = FsCluster::get_instance(configuration);
}

// plugin configuration manager
auto get_type_identifier = [](auto &fs_handler) -> std::string { return fs_handler->get_type(); };

void FsHandler::env_configuration_preamble(PluginConfiguration &pc)
{
  pc.properties.clear();

  pc.properties.insert({"AGORA_PROPERTIES_CONTAINER_NAME", description_fs->get_properties_name(pc.app_id)});

  pc.properties.insert({"DESCRIPTION_FS_TYPE", get_type_identifier(description_fs)});
  pc.properties.insert({"DOE_FS_TYPE", get_type_identifier(doe_fs)});
  pc.properties.insert({"PREDICTION_FS_TYPE", get_type_identifier(prediction_fs)});
  pc.properties.insert({"CLUSTER_FS_TYPE", get_type_identifier(cluster_fs)});
  pc.properties.insert({"OBSERVATION_FS_TYPE", get_type_identifier(observation_fs)});

  // pc.properties.insert({"DATABASE_ADDRESS", get_address()});
  // pc.properties.insert({"DATABASE_USERNAME", get_db_username()});
  // pc.properties.insert({"DATABASE_PASSWORD", get_db_password()});
}

template <> void FsHandler::create_env_configuration<PluginType::DOE>(PluginConfiguration &pc)
{
  env_configuration_preamble(pc);

  pc.properties.insert({"KNOBS_CONTAINER_NAME", description_fs->get_knobs_name(pc.app_id)});
  pc.properties.insert({"DOE_CONTAINER_NAME", doe_fs->get_doe_name(pc.app_id)});
  pc.properties.insert({"DOE_PARAMETERS_CONTAINER_NAME", description_fs->get_doe_parameters_name(pc.app_id)});
  pc.properties.insert({"TOTAL_CONFIGURATIONS_CONTAINER_NAME", doe_fs->get_total_configurations_name(pc.app_id)});
}

template <> void FsHandler::create_env_configuration<PluginType::Model>(PluginConfiguration &pc)
{
  if (pc.metric_name.empty())
    throw std::runtime_error("PluginConfiguration: the metric name must be specified");
  if (pc.iteration_number < 0)
    throw std::runtime_error("PluginConfiguration: the iteration number must be specified and cannot be negative");

  env_configuration_preamble(pc);

  pc.properties.insert({"ITERATION_NUMBER", std::to_string(pc.iteration_number)});
  pc.properties.insert({"METRIC_NAME", pc.metric_name});
  pc.properties.insert({"KNOBS_CONTAINER_NAME", description_fs->get_knobs_name(pc.app_id)});
  pc.properties.insert({"FEATURES_CONTAINER_NAME", description_fs->get_features_name(pc.app_id)});
  pc.properties.insert({"OBSERVATION_CONTAINER_NAME", observation_fs->get_observation_name(pc.app_id)});

  pc.properties.insert({"MODEL_CONTAINER_NAME", model_fs->get_model_name(pc.app_id, pc.metric_name)});
  pc.properties.insert({"MODEL_PARAMETERS_CONTAINER_NAME", description_fs->get_model_parameters_name(pc.app_id, pc.metric_name)});
}

template <> void FsHandler::create_env_configuration<PluginType::Cluster>(PluginConfiguration &pc)
{
  env_configuration_preamble(pc);

  pc.properties.insert({"FEATURES_CONTAINER_NAME", description_fs->get_features_name(pc.app_id)});
  pc.properties.insert({"OBSERVATION_CONTAINER_NAME", observation_fs->get_observation_name(pc.app_id)});
  pc.properties.insert({"CLUSTER_CONTAINER_NAME", cluster_fs->get_cluster_name(pc.app_id)});
  pc.properties.insert({"CLUSTER_PARAMETERS_CONTAINER_NAME", description_fs->get_clustering_parameters_name(pc.app_id)});
}

template <> void FsHandler::create_env_configuration<PluginType::Prediction>(PluginConfiguration &pc)
{
  env_configuration_preamble(pc);

  pc.properties.insert({"KNOBS_CONTAINER_NAME", description_fs->get_knobs_name(pc.app_id)});
  pc.properties.insert({"METRICS_CONTAINER_NAME", description_fs->get_metrics_name(pc.app_id)});
  pc.properties.insert({"FEATURES_CONTAINER_NAME", description_fs->get_features_name(pc.app_id)});
  pc.properties.insert({"TOTAL_CONFIGURATIONS_CONTAINER_NAME", doe_fs->get_total_configurations_name(pc.app_id)});
  pc.properties.insert({"CLUSTER_CONTAINER_NAME", cluster_fs->get_cluster_name(pc.app_id)});
  pc.properties.insert({"PREDICTIONS_CONTAINER_NAME", prediction_fs->get_prediction_name(pc.app_id)});

  pc.properties.insert({"MODELS_CONTAINER", model_fs->get_models_path(pc.app_id)});
}

} // namespace agora
