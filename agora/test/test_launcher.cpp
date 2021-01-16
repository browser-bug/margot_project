#include <filesystem>

#include "agora/application_manager.hpp"
#include "agora/launcher.hpp"
#include "agora/launcher_configuration.hpp"
#include "agora/plugin_configuration.hpp"

using namespace agora;
namespace fs = std::filesystem;

int main()
{
  // create the application manager
  agora::ApplicationManager &app_manager = agora::ApplicationManager::get_instance();

  // setup the logger
  agora::LoggerConfiguration log_config(LogLevel::DEBUG);
  app_manager.setup_logger(log_config);

  auto logger = app_manager.get_logger();

  const fs::path plugins_path = "/home/bernardo/Development/margot/agora/plugins";
  const fs::path working_dir = "/home/bernardo/Development/margot/agora/test/temp_dir";
  const fs::path data_dir = "/home/bernardo/Development/margot/agora/data";
  fs::create_directory(working_dir);

  LauncherConfiguration config(plugins_path, working_dir);
  application_id app_id("app1", 1, "block1");

  ////// DOE ///////
  //auto launcher = Launcher::get_launcher(config, "doe");

  //PluginConfiguration doe_config(app_id);
  //doe_config.properties.insert({"APPLICATION_NAME", app_id.app_name});
  //doe_config.properties.insert({"BLOCK_NAME", app_id.block_name});
  //doe_config.properties.insert({"AGORA_PROPERTIES_CONTAINER_NAME", data_dir / "descriptions/bar/foo_agora_properties.csv"});

  //doe_config.properties.insert({"DESCRIPTION_FS_TYPE", "csv"});
  //doe_config.properties.insert({"DOE_FS_TYPE", "csv"});
  //doe_config.properties.insert({"PREDICTION_FS_TYPE", "csv"});
  //doe_config.properties.insert({"CLUSTER_FS_TYPE", "csv"});
  //doe_config.properties.insert({"OBSERVATION_FS_TYPE", "csv"});

  //doe_config.properties.insert({"KNOBS_CONTAINER_NAME", data_dir / "descriptions/bar/foo_knobs.csv"});
  //doe_config.properties.insert({"DOE_CONTAINER_NAME", working_dir / "doe.csv"});
  //doe_config.properties.insert({"DOE_PARAMETERS_CONTAINER_NAME", data_dir / "descriptions/bar/foo_doe_parameters.csv"});
  //doe_config.properties.insert({"TOTAL_CONFIGURATIONS_CONTAINER_NAME", working_dir / "total_configs.csv"});

  //launcher->initialize_workspace(app_id);
  //launcher->set_plugin_configuration(doe_config);
  //launcher->launch();
  //launcher->wait();

  ///// CLUSTER ///////
  //auto launcher = Launcher::get_launcher(config, "cluster");

  //PluginConfiguration cluster_config(app_id);
  //cluster_config.properties.insert({"APPLICATION_NAME", app_id.app_name});
  //cluster_config.properties.insert({"BLOCK_NAME", app_id.block_name});
  //cluster_config.properties.insert({"AGORA_PROPERTIES_CONTAINER_NAME", data_dir / "descriptions/bar/foo_agora_properties.csv"});

  //cluster_config.properties.insert({"DESCRIPTION_FS_TYPE", "csv"});
  //cluster_config.properties.insert({"DOE_FS_TYPE", "csv"});
  //cluster_config.properties.insert({"PREDICTION_FS_TYPE", "csv"});
  //cluster_config.properties.insert({"CLUSTER_FS_TYPE", "csv"});
  //cluster_config.properties.insert({"OBSERVATION_FS_TYPE", "csv"});

  //cluster_config.properties.insert({"FEATURES_CONTAINER_NAME", data_dir / "descriptions/bar/foo_features.csv"});
  //cluster_config.properties.insert({"OBSERVATION_CONTAINER_NAME", data_dir / "observations/bar/foo_observations.csv"});
  //cluster_config.properties.insert({"CLUSTER_CONTAINER_NAME", working_dir / "cluster.csv"});
  //cluster_config.properties.insert({"CLUSTER_PARAMETERS_CONTAINER_NAME", data_dir / "descriptions/bar/foo_clustering_parameters.csv"});

  //launcher->initialize_workspace(app_id);
  //launcher->set_plugin_configuration(cluster_config);
  //launcher->launch();
  //launcher->wait();

  ///// MODEL /////////
  //auto launcher = Launcher::get_launcher(config, "model");

  //PluginConfiguration model_config(app_id, "quality", 29);
  //model_config.properties.insert({"APPLICATION_NAME", app_id.app_name});
  //model_config.properties.insert({"BLOCK_NAME", app_id.block_name});
  //model_config.properties.insert({"AGORA_PROPERTIES_CONTAINER_NAME", data_dir / "descriptions/bar/foo_agora_properties.csv"});

  //model_config.properties.insert({"DESCRIPTION_FS_TYPE", "csv"});
  //model_config.properties.insert({"DOE_FS_TYPE", "csv"});
  //model_config.properties.insert({"PREDICTION_FS_TYPE", "csv"});
  //model_config.properties.insert({"CLUSTER_FS_TYPE", "csv"});
  //model_config.properties.insert({"OBSERVATION_FS_TYPE", "csv"});

  //model_config.properties.insert({"ITERATION_NUMBER", "29"});
  //model_config.properties.insert({"METRIC_NAME", "quality"});
  //model_config.properties.insert({"KNOBS_CONTAINER_NAME", data_dir / "descriptions/bar/foo_knobs.csv"});
  //model_config.properties.insert({"FEATURES_CONTAINER_NAME", data_dir / "descriptions/bar/foo_features.csv"});
  //model_config.properties.insert({"OBSERVATION_CONTAINER_NAME", data_dir / "observations/bar/foo_observations.csv"});
  //model_config.properties.insert({"MODEL_CONTAINER_NAME", working_dir / "quality_model.data"});
  //model_config.properties.insert({"MODEL_PARAMETERS_CONTAINER_NAME", data_dir / "descriptions/bar/foo_quality_model_parameters.csv"});

  //launcher->initialize_workspace(app_id);
  //launcher->set_plugin_configuration(model_config);
  //launcher->launch();
  //launcher->wait();

  ///// PREDICTION /////////
  auto launcher = Launcher::get_instance(config, "predict");

  PluginConfiguration prediction_config(app_id);
  prediction_config.properties.insert({"APPLICATION_NAME", app_id.app_name});
  prediction_config.properties.insert({"BLOCK_NAME", app_id.block_name});
  prediction_config.properties.insert({"AGORA_PROPERTIES_CONTAINER_NAME", data_dir / "descriptions/bar/foo_agora_properties.csv"});

  prediction_config.properties.insert({"DESCRIPTION_FS_TYPE", "csv"});
  prediction_config.properties.insert({"DOE_FS_TYPE", "csv"});
  prediction_config.properties.insert({"PREDICTION_FS_TYPE", "csv"});
  prediction_config.properties.insert({"CLUSTER_FS_TYPE", "csv"});
  prediction_config.properties.insert({"OBSERVATION_FS_TYPE", "csv"});
  prediction_config.properties.insert({"TOTAL_CONFIGURATIONS_CONTAINER_NAME", working_dir / "total_configs.csv"});
  prediction_config.properties.insert({"CLUSTER_CONTAINER_NAME", working_dir / "cluster.csv"});
  prediction_config.properties.insert({"PREDICTIONS_CONTAINER_NAME", working_dir / "predictions.csv"});
  prediction_config.properties.insert({"MODEL_CONTAINERS", working_dir});

  prediction_config.properties.insert({"KNOBS_CONTAINER_NAME", data_dir / "descriptions/bar/foo_knobs.csv"});
  prediction_config.properties.insert({"FEATURES_CONTAINER_NAME", data_dir / "descriptions/bar/foo_features.csv"});
  prediction_config.properties.insert({"METRIC_CONTAINER_NAME", data_dir / "descriptions/bar/foo_metrics.csv"});

  launcher->initialize_workspace(app_id);
  launcher->set_plugin_configuration(prediction_config);
  launcher->launch();
  launcher->wait();
}
