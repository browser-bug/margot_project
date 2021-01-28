#include <regex>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/property_tree/json_parser.hpp>

#include <heel/model_application.hpp>
#include <heel/parser_application.hpp>
#include <heel/parser_operating_point.hpp>
#include <heel/composer_utils.hpp>

#include "agora/application_handler.hpp"
#include "agora/application_manager.hpp"

using namespace agora;
namespace pt = boost::property_tree;

RemoteApplicationHandler::RemoteApplicationHandler(const application_id &application_id, const FsConfiguration &fs_config,
                                                   const LauncherConfiguration &launcher_config)
    : app_id(application_id), LOG_HEADER("Handler " + application_id.str() + ": "), status(ApplicationStatus::CLUELESS),
      iteration_number(0), num_configurations_sent_per_iteration(0)
{
  ApplicationManager &am = ApplicationManager::get_instance();
  logger = am.get_logger();
  remote = am.get_remote_handler();

  fs_handler = FsHandler::get_instance(fs_config);

  // set a general configuration for every type of plugin launcher
  launcher_configuration = launcher_config;
}

RemoteApplicationHandler::~RemoteApplicationHandler()
{
  fs_handler->erase(app_id);

  static auto clear = [](const auto &launcher) {
    if (launcher)
    {
      launcher->clear_workspace();
    }
  };
  clear(doe_launcher);
  clear(prediction_launcher);
  clear(cluster_launcher);
  clear(doe_launcher);
  for (const auto &launcher : model_launchers)
  {
      clear(launcher.second);
  }
}

void RemoteApplicationHandler::welcome_client(const client_id_t &cid, const std::string &info)
{
  // we lock earlier because we have to check all different possible states to get a clue of what's going on
  std::unique_lock<std::mutex> lock(app_mutex);

  // this is a new client so we register it
  active_clients.insert(cid);

  // the handler has just been created, so this is the first client that has been received
  if (status & ApplicationStatus::CLUELESS)
  {
    if(!parse_informations(info))
    {
      logger->warning(LOG_HEADER, "couldn't find a corresponding application description. Aborting.");
      return;
    }

    logger->info(LOG_HEADER, "storing description informations.");
    fs_handler->store_description(app_id, description);

    initialize_plugin_launchers();

    status = ApplicationStatus::RECOVERING;
    lock.unlock();
    start_recovering();
    lock.lock();
  }

  // we're building the doe so just wait for a configuration to explore
  if (status & ApplicationStatus::BUILDING_DOE)
  {
    logger->info(LOG_HEADER, "building a new DOE, wait for a configuration to explore.");
  }

  // we're building the models and/or the clusters so just wait for more configs to explore or the predictions
  if (status & (ApplicationStatus::BUILDING_MODEL | ApplicationStatus::BUILDING_CLUSTER))
  {
    logger->info(LOG_HEADER, "building models and/or cluster, wait for the predictions or more configurations to explore.");
  }

  // we're currently trying to recover data (if any) for this application, just wait new orders
  if (status & ApplicationStatus::RECOVERING)
  {
    logger->info(LOG_HEADER, "trying to recover data (if any) from storage. Please stand by.");
  }

  // we have already a configuration available and we can start exploring
  if (status & ApplicationStatus::EXPLORING)
  {
    logger->info(LOG_HEADER, "sending a new configuration to explore.");
    send_configuration(cid);
  }

  // we already have a prediction available and we can share to the new client the application knowledge
  if (status & ApplicationStatus::WITH_PREDICTION)
  {
    logger->info(LOG_HEADER, "sending the application knowledge to the new client.");
    send_prediction(cid);
  }

  // we don't know what's going on anymore and we can't procede further, try to process app information again
  if (status & ApplicationStatus::UNDEFINED)
  {
    logger->info(LOG_HEADER, "undefined status, aborting the online autotuning procedure.");
    send_abort_message(cid);
  }
}

void RemoteApplicationHandler::bye_client(const client_id_t &cid)
{
  logger->info(LOG_HEADER, "terminating connection with client \"", cid, "\".");

  std::unique_lock<std::mutex> lock(app_mutex);
  active_clients.erase(cid);

  // it was the last client and we can safely clear everything
  if (active_clients.empty() &&
      !(status & (ApplicationStatus::BUILDING_CLUSTER | ApplicationStatus::BUILDING_MODEL | ApplicationStatus::BUILDING_DOE |
                  ApplicationStatus::BUILDING_PREDICTION | ApplicationStatus::EXPLORING | ApplicationStatus::RECOVERING)))
  {
    logger->info(LOG_HEADER, cid, " was the last client and I'm not busy. Clearing internal data infos.");
    status = ApplicationStatus::CLUELESS;
    iteration_number = 0;
    num_configurations_sent_per_iteration = 0;
  }
}

void RemoteApplicationHandler::process_observation(const client_id_t &cid, const long duration_sec, const long duration_ns,
                                                   const std::string &observation_values)
{
  std::unique_lock<std::mutex> lock(app_mutex);

  // if we're not exploring configurations, we're not interested in this message type
  if (!(status & ApplicationStatus::EXPLORING))
  {
    logger->warning(LOG_HEADER, "the DSE phase has ended, ignoring the new observation.");
    return;
  }

  margot::heel::block_model op_description;
  if (!parse_observation(observation_values, op_description))
  {
    logger->warning(LOG_HEADER, "the operating point received failed to be parsed.");
    return;
  }

  //TODO: we could add the observation even if we're not exploring since it is useful anyway and harmless.
  fs_handler->insert_observation_entry(app_id, cid, duration_sec, duration_ns, op_description.ops.front());

  // if we still have some explorations to do we can't continue yet
  if (!doe.required_explorations.empty() && num_configurations_sent_per_iteration <= num_configurations_per_iteration)
  {
    send_configuration(cid);
    return;
  }

  iteration_number++;
  num_configurations_sent_per_iteration = 0;
  status = ApplicationStatus::BUILDING_MODEL | ApplicationStatus::BUILDING_CLUSTER;
  lock.unlock();

  logger->info(LOG_HEADER, "starting the modeling phase.");
  std::vector<pid_t> model_pids = start_modeling();

  if (are_features_enabled())
  {
    logger->info(LOG_HEADER, "starting the clustering phase.");
    pid_t cluster_pid_t = start_clustering();
    Launcher::wait(cluster_pid_t);
  }

  // wait for the models generation
  for (const auto &pid : model_pids)
  {
    Launcher::wait(pid);
  }

  lock.lock();
  // retrieve the cluster informaton
  if (are_features_enabled())
  {
    cluster = fs_handler->load_cluster(app_id, description);
    if (is_cluster_valid())
    {
      status |= ApplicationStatus::WITH_CLUSTER;
    }
  }

  // check if the models were valid for each metric
  if (are_models_valid())
  {
    status |= ApplicationStatus::WITH_MODEL;
  }

  // if everything went fine, we can generate the application knowledge
  if ((status & ApplicationStatus::WITH_MODEL) && (are_features_enabled() ? bool((status & ApplicationStatus::WITH_CLUSTER)) : true))
  {
    lock.unlock();

    // launch the prediction plugin
    logger->info(LOG_HEADER, "starting the prediction phase.");

    pid_t prediction_pid_t = start_prediction();
    Launcher::wait(prediction_pid_t);

    lock.lock();
    // retrieve the final predictions
    prediction = fs_handler->load_prediction(app_id, description);
    if (is_prediction_valid())
    {
      logger->info(LOG_HEADER, "the application knowledge has been retrieved.");
      status = ApplicationStatus::WITH_PREDICTION;
      // finally broadcast the application knowledge and exit
      broadcast_prediction();
      return;
    } else
    {
      logger->info(LOG_HEADER, "the application knowledge was empty. Restarting the DSE phase.");
    }
  }

  // some of the models are not valid so we need to collect more observations
  if (doe.required_explorations.empty())
  {
    status = ApplicationStatus::BUILDING_DOE;
    lock.unlock();

    logger->info(LOG_HEADER, "no more configurations avilable.");

    // call the doe plugin (with the last configuration used) and wait for its completion
    logger->info(LOG_HEADER, "starting the DOE generation process once again.");
    pid_t doe_pid_t = doe_launcher->launch();
    Launcher::wait(doe_pid_t);

    lock.lock();
    // retrieve the doe informations produced
    doe = fs_handler->load_doe(app_id, description);
  }

  // start the DSE phase
  if (!is_doe_valid())
  {
    logger->warning(LOG_HEADER, "no configuration to explore.");
    return;
  }
  status = ApplicationStatus::WITH_DOE;

  if (active_clients.empty())
  {
    logger->warning(LOG_HEADER, "no more active clients.");
    return;
  }

  logger->info(LOG_HEADER, "starting the Design Space Exploration.");
  status |= ApplicationStatus::EXPLORING;
  // send the available configurations to the active clients
  for (const auto &cid : active_clients)
  {
    send_configuration(cid);
  }
}

void RemoteApplicationHandler::start_recovering()
{
  logger->info(LOG_HEADER, "trying to recover data from storage first.");

  doe = fs_handler->load_doe(app_id, description);
  prediction = fs_handler->load_prediction(app_id, description);
  if(are_features_enabled()){
    cluster = fs_handler->load_cluster(app_id, description);
  }

  std::unique_lock<std::mutex> lock(app_mutex);

  if (is_prediction_valid())
  {
    logger->info(LOG_HEADER, "recovered predictions from storage.");
    status = ApplicationStatus::WITH_PREDICTION;
    return;
  }

  if (are_models_valid() && (are_features_enabled() ? is_cluster_valid() : true))
  {
    logger->info(LOG_HEADER, "recovered models", (are_features_enabled() ? " and clusters" : ""), " from storage.");
    status = ApplicationStatus::WITH_MODEL | ApplicationStatus::WITH_CLUSTER;
    lock.unlock();

    logger->info(LOG_HEADER, "starting the prediction phase.");

    pid_t prediction_pid_t = start_prediction();
    Launcher::wait(prediction_pid_t);

    lock.lock();
    // retrieve the final predictions
    prediction = fs_handler->load_prediction(app_id, description);
    if (is_prediction_valid())
    {
      logger->info(LOG_HEADER, "the application knowledge has been retrieved.");
      status = ApplicationStatus::WITH_PREDICTION;
      // finally broadcast the application knowledge and exit
      broadcast_prediction();
      return;
    }
    logger->info(LOG_HEADER, "the application knowledge was empty.");
  }

  if (is_doe_valid())
  {
    logger->info(LOG_HEADER, "recovered doe configurations from storage. Restaring the Design Space Exploration.");
    status = ApplicationStatus::EXPLORING | ApplicationStatus::WITH_DOE;
    return;
  }

  logger->info(LOG_HEADER, "starting the DOE building phase.");
  status = ApplicationStatus::BUILDING_DOE;
  lock.unlock();

  pid_t doe_pid = start_doe();
  Launcher::wait(doe_pid);
  doe = fs_handler->load_doe(app_id, description);

  lock.lock();
  if (!is_doe_valid())
  {
    logger->warning(LOG_HEADER, "the DOE plugin couldn't produce anything. Unstable.");
    status = ApplicationStatus::UNDEFINED;
    return;
  }

  logger->info(LOG_HEADER, "starting the Design Space Exploration.");
  status = ApplicationStatus::EXPLORING | ApplicationStatus::WITH_DOE;

  logger->info(LOG_HEADER, "creating the observations container into the storage.");
  fs_handler->create_observation_table(app_id, description);

  // send the available configurations to the active clients
  for (const auto &client_name : active_clients)
  {
    send_configuration(client_name);
  }
}

const std::string RemoteApplicationHandler::configuration_to_json(const configuration_model &configuration) const
{
  pt::ptree op;
  pt::ptree knobs;
  for (const auto &knob : description.knobs)
  {
    knobs.put(knob.name, (knob.type == "string") ? ("\"" + configuration.at(knob.name) + "\"") : configuration.at(knob.name));
  }
  op.add_child("knobs", knobs);

  if(are_features_enabled())
  {
    pt::ptree features;
    for(const auto &feature : description.features.fields)
    {
      features.put(feature.name, 9999);
    }
    op.add_child("features", features);
  }

  pt::ptree metrics;
  for(const auto& metric : description.metrics)
  {
    if(metric.distribution)
    {
      margot::heel::add_list(metrics, std::vector{"9999", "0"}, metric.name);
    } else
    {
      metrics.put(metric.name, 9999);
    }
  }
  op.add_child("metrics", metrics);

  pt::ptree op_list;
  op_list.push_back({"", op});
  pt::ptree root;
  root.add_child(description.name, op_list);

  std::stringstream ss;
  pt::json_parser::write_json(ss, root, false);
  std::regex reg("\"([0-9]+\\.?[0-9]*)\"|\n");
  return std::regex_replace(ss.str(), reg, "$1");
}

const std::string RemoteApplicationHandler::prediction_to_json(const prediction_model &prediction) const
{
  pt::ptree op_list;

  for (const auto &result : prediction.predicted_results)
  {
    pt::ptree op;
    const auto pred_id = result.first;

    if (are_features_enabled())
    {
      pt::ptree features;
      for (const auto &feature : description.features.fields)
      {
        features.put(feature.name, prediction.features.at(pred_id).at(feature.name));
      }
      op.add_child("features", features);
    }

    pt::ptree knobs;
    for (const auto &knob : description.knobs)
    {
      knobs.put(knob.name, (knob.type == "string") ? ("\"" + prediction.configurations.at(pred_id).at(knob.name) + "\"")
                                                   : prediction.configurations.at(pred_id).at(knob.name));
    }
    op.add_child("knobs", knobs);

    pt::ptree metrics;
    for (const auto &metric : description.metrics)
    {
      if (metric.distribution)
      {
        margot::heel::add_list(metrics,
                               std::vector{prediction.predicted_results.at(pred_id).at(metric.name)._avg,
                                           prediction.predicted_results.at(pred_id).at(metric.name)._std},
                               metric.name);
      } else
      {
        metrics.put(metric.name, prediction.predicted_results.at(pred_id).at(metric.name)._avg);
      }
    }
    op.add_child("metrics", metrics);

    op_list.push_back({"", op});
  }

  pt::ptree root;
  root.add_child(description.name, op_list);

  std::stringstream ss;
  pt::json_parser::write_json(ss, root, false);
  std::regex reg("\"([0-9]+\\.?[0-9]*)\"|\n");
  return std::regex_replace(ss.str(), reg, "$1");
}

pid_t RemoteApplicationHandler::start_doe()
{
  logger->info(LOG_HEADER, "creating the doe plugin configuration file.");
  PluginConfiguration doe_config("plugin_config.env", app_id);
  fs_handler->create_env_configuration<PluginType::DOE>(doe_config);

  logger->info(LOG_HEADER, "starting the DOE generation process.");
  return doe_launcher->launch(doe_config);
}

std::vector<pid_t> RemoteApplicationHandler::start_modeling()
{
  // creating the model plugin configuration file for each metric
  logger->info(LOG_HEADER, "creating a model plugin configuration file for each metric.");
  std::unordered_map<std::string, std::pair<std::string, PluginConfiguration>> model_configs;
  for (const auto &metric : description.metrics)
  {
    PluginConfiguration model_config("plugin_config.env", app_id, metric.name, iteration_number);
    fs_handler->create_env_configuration<PluginType::Model>(model_config);
    model_configs.insert({metric.name, {metric.prediction_plugin, model_config}});
  }

  // call the model plugin for each metric
  std::vector<pid_t> model_pids;
  model_pids.reserve(model_configs.size());
  for (const auto &itr : model_configs)
  {
    auto plugin_name = itr.second.first;
    const auto &plugin_config = itr.second.second;
    const auto &model_launcher = model_launchers.at(plugin_name);

    logger->info(LOG_HEADER, "starting the model generation process for the metric [", plugin_config.metric_name, "].");
    pid_t model_pid_t = model_launcher->launch(plugin_config);
    model_pids.push_back(model_pid_t);
  }

  return model_pids;
}

pid_t RemoteApplicationHandler::start_clustering()
{
  logger->info(LOG_HEADER, "creating the cluster plugin configuration file");
  PluginConfiguration cluster_config("plugin_config.env", app_id);
  fs_handler->create_env_configuration<PluginType::Cluster>(cluster_config);

  // call the cluster plugin
  logger->info(LOG_HEADER, "starting the cluster generation process.");
  return cluster_launcher->launch(cluster_config);
}

pid_t RemoteApplicationHandler::start_prediction()
{
  logger->info(LOG_HEADER, "creating the prediction plugin configuration file");
  PluginConfiguration prediction_config("plugin_config.env", app_id);
  fs_handler->create_env_configuration<PluginType::Prediction>(prediction_config);

  // call the cluster plugin and wait for its completion
  logger->info(LOG_HEADER, "starting the application knowledge generation process.");
  return prediction_launcher->launch(prediction_config);
}

bool RemoteApplicationHandler::parse_informations(const std::string &info)
{
  logger->info(LOG_HEADER, "parsing new app informations.");

  margot::heel::application_model app_description;
  pt::ptree app_description_node;
  std::stringstream info_stream(info);
  pt::read_json(info_stream, app_description_node);
  margot::heel::parse(app_description, app_description_node);

  // store informations into the relative block description object
  auto block_itr = std::find_if(app_description.blocks.begin(), app_description.blocks.end(), [&](const auto &block) {
    return (app_description.name == app_id.app_name && app_description.version == app_id.version && block.name == app_id.block_name);
  });
  if (block_itr == app_description.blocks.end())
  {
    return false;
  }
  description = *block_itr;
  num_configurations_per_iteration = std::stoi(description.agora.number_configurations_per_iteration);
  status = ApplicationStatus::WITH_INFORMATION;
  return true;
}

bool RemoteApplicationHandler::parse_observation(const std::string &observation_values, margot::heel::block_model& op_description)
{
  op_description.name = description.name;
  op_description.knobs = description.knobs;
  op_description.features.fields = description.features.fields;
  op_description.metrics = description.metrics;

  pt::ptree op_node;
  std::stringstream op_stream(observation_values);
  pt::read_json(op_stream, op_node);
  margot::heel::parse_operating_points(op_description, op_node);

  return !op_description.ops.empty();
}

void RemoteApplicationHandler::initialize_plugin_launchers()
{
  logger->info(LOG_HEADER, "initializing the plugin launchers.");

  doe_launcher = Launcher::get_instance(launcher_configuration, description.agora.doe_plugin);
  doe_launcher->initialize_workspace(app_id);

  if (are_features_enabled())
  {
    cluster_launcher = Launcher::get_instance(launcher_configuration, description.agora.clustering_plugin);
    cluster_launcher->initialize_workspace(app_id);
  }

  // TODO: "predict" is a value that needs to be specified on margot_heel configuration file, for now let's use a default
  prediction_launcher = Launcher::get_instance(launcher_configuration, "predict");
  prediction_launcher->initialize_workspace(app_id);

  // adding a new launcher for every metric we have
  for (const auto &metric : description.metrics)
  {
    // we could have the same plugin launcher for two different metrics, so we have to check on that
    if (model_launchers.find(metric.prediction_plugin) != model_launchers.end())
      continue;
    auto model_launcher = Launcher::get_instance(launcher_configuration, metric.prediction_plugin);
    model_launcher->initialize_workspace(app_id);
    model_launchers.insert({metric.prediction_plugin, std::shared_ptr<Launcher>(std::move(model_launcher))});
  }
}
