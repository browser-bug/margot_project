#include <regex>
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#include <boost/property_tree/json_parser.hpp>

#include <heel/composer_utils.hpp>
#include <heel/model_application.hpp>
#include <heel/parser_application.hpp>
#include <heel/parser_operating_point.hpp>

#include "agora/application_handler.hpp"
#include "agora/application_manager.hpp"

using namespace agora;
namespace pt = boost::property_tree;

RemoteApplicationHandler::RemoteApplicationHandler(const application_id &application_id, const FsConfiguration &fs_config,
                                                   const LauncherConfiguration &launcher_config)
    : app_id(application_id), LOG_HEADER("Handler " + application_id.str() + ": "), handler_status(InternalStatus::CLUELESS),
      iteration_number(0), num_configurations_per_iteration(0), num_configurations_sent_per_iteration(0)
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
  clear_launchers();
}

void RemoteApplicationHandler::welcome_client(const client_id_t &cid, const std::string &info)
{
  std::unique_lock<std::mutex> lock(app_mutex);

  // this is a new client so we register it
  add_client(cid);

  // the handler has just been created, so this is the first client that has been received
  if (check_state(InternalStatus::CLUELESS))
  {
    if (!parse_informations(info, description))
    {
      logger->warning(LOG_HEADER, "couldn't find a corresponding application description. Aborting.");
      return;
    }

    logger->info(LOG_HEADER, "storing description informations.");
    fs_handler->store_description(app_id, description);

    initialize_plugin_launchers();

    start_recovering();
  }

  // we're building the doe so just wait for a configuration to explore
  if (check_state(InternalStatus::BUILDING_DOE))
  {
    logger->info(LOG_HEADER, "building a new DOE, wait for a configuration to explore.");
    return;
  }

  // we're building the models and/or the clusters so just wait for more configs to explore or the predictions
  if (check_state(InternalStatus::BUILDING_MODEL) || check_state(InternalStatus::BUILDING_CLUSTER))
  {
    logger->info(LOG_HEADER, "building models and/or cluster, wait for the predictions or more configurations to explore.");
    return;
  }

  // we're building the predictions so just wait for the broadcasting
  if (check_state(InternalStatus::BUILDING_PREDICTION))
  {
    logger->info(LOG_HEADER, "building predictions, wait for the application knowledge.");
    return;
  }

  // we're currently trying to recover data (if any) for this application, just wait new orders
  if (check_state(InternalStatus::RECOVERING))
  {
    logger->info(LOG_HEADER, "trying to recover data (if any) from storage. Please stand by.");
    return;
  }

  // we have already a configuration available and we can start exploring
  if (check_state(InternalStatus::EXPLORING))
  {
    logger->info(LOG_HEADER, "sending a new configuration to explore.");
    send_configuration(cid);
    return;
  }

  // we already have a prediction available and we can share to the new client the application knowledge
  if (check_state(InternalStatus::WITH_PREDICTION))
  {
    logger->info(LOG_HEADER, "sending the application knowledge to the new client.");
    send_prediction(cid);
    return;
  }

  // we don't know what's going on anymore and we can't proceed further, try to process app information again
  if (check_state(InternalStatus::UNDEFINED))
  {
    logger->info(LOG_HEADER, "undefined status, aborting the online autotuning procedure.");
    send_abort_message(cid);
  }
}

void RemoteApplicationHandler::bye_client(const client_id_t &cid)
{
  std::unique_lock<std::mutex> lock(app_mutex);

  logger->info(LOG_HEADER, "terminating connection with client \"", cid, "\".");

  remove_client(cid);
  if (active_clients.empty())
  {
    // if it was the last client we free up the memory used
    logger->info(LOG_HEADER, cid, " was the last client. Freeing up the memory.");
    set_state(InternalStatus::CLUELESS);
    doe.clear();
    cluster.clear();
    prediction.clear();
  }
}

void RemoteApplicationHandler::process_observation(const client_id_t &cid, const long duration_sec, const long duration_ns,
                                                   const std::string &observation_values)
{
  std::unique_lock<std::mutex> lock(app_mutex);

  // if we're not exploring configurations, we're not interested in this message type
  if (!check_state(InternalStatus::EXPLORING))
  {
    logger->warning(LOG_HEADER, "the DSE phase has ended, ignoring the new observation.");
    return;
  }
  lock.unlock();

  margot::heel::block_model op_description;
  if (!parse_observation(observation_values, op_description))
  {
    logger->warning(LOG_HEADER, "parsing error, ignoring the new observation.");
    return;
  }

  fs_handler->insert_observation_entry(app_id, cid, duration_sec, duration_ns, op_description.ops.front());

  // if we still have some explorations to do we can't continue yet
  lock.lock();
  if (is_doe_valid() && num_configurations_sent_per_iteration < num_configurations_per_iteration)
  {
    if(!send_configuration(cid)){
      logger->info(LOG_HEADER, "there was a problem sending configuration to client ", cid, "... trying again.");
    }
    return;
  }

  // update internal infos
  iteration_number++;
  num_configurations_sent_per_iteration = 0;

  logger->info(LOG_HEADER, "starting the modeling phase.");
  std::vector<pid_t> pids_to_wait = start_modeling();

  if (are_features_enabled())
  {
    logger->info(LOG_HEADER, "starting the clustering phase.");
    pid_t cluster_pid = start_clustering();
    pids_to_wait.push_back(cluster_pid);
  }

  // wait for the plugins to finish
  lock.unlock();
  for (const auto &pid : pids_to_wait)
  {
    Launcher::wait(pid);
  }
  lock.lock();

  // check the data produced
  if (are_models_valid())
  {
    set_state(InternalStatus::WITH_MODEL);
  }
  if (are_features_enabled())
  {
    cluster = fs_handler->load_cluster(app_id, description);
    if (is_cluster_valid())
    {
      set_state(InternalStatus::WITH_CLUSTER, false);
    }
  }

  // if everything went fine, we can generate the application knowledge
  if (are_features_enabled() ? (check_state(InternalStatus::WITH_MODEL) && check_state(InternalStatus::WITH_CLUSTER))
                             : check_state(InternalStatus::WITH_MODEL))
  {
    // launch the prediction plugin
    logger->info(LOG_HEADER, "starting the prediction phase.");
    pid_t prediction_pid = start_prediction();
    lock.unlock();
    Launcher::wait(prediction_pid);
    lock.lock();
    prediction = fs_handler->load_prediction(app_id, description);

    if (is_prediction_valid())
    {
      logger->info(LOG_HEADER, "the application knowledge has been retrieved.");
      set_state(InternalStatus::WITH_PREDICTION);
      // finally broadcast the application knowledge and exit
      broadcast_prediction();
      return;
    }
    logger->info(LOG_HEADER, "the application knowledge was empty. Restarting the DSE phase.");
  }

  // some of the models are not valid so we need to collect more observations
  if (!is_doe_valid())
  {
    logger->info(LOG_HEADER, "no more configurations avilable.");

    // call the doe plugin (with the last configuration used) and wait for its completion
    logger->info(LOG_HEADER, "starting the DOE generation process once again.");
    pid_t doe_pid_t = start_doe();
    lock.unlock();
    Launcher::wait(doe_pid_t);
    lock.lock();
    doe = fs_handler->load_doe(app_id, description);
    if (!is_doe_valid())
    {
      logger->warning(LOG_HEADER, "no configuration to explore.");
      set_state(InternalStatus::UNDEFINED);
      return;
    }
  }

  // start the DSE phase
  set_state(InternalStatus::WITH_DOE | InternalStatus::EXPLORING);

  logger->info(LOG_HEADER, "starting the Design Space Exploration.");

  // send the available configurations to the active clients
  for (const auto &cid : active_clients)
  {
    send_configuration(cid);
  }
}

void RemoteApplicationHandler::start_recovering()
{
  logger->info(LOG_HEADER, "trying to recover data from storage first.");
  set_state(InternalStatus::RECOVERING);

  doe = fs_handler->load_doe(app_id, description);
  prediction = fs_handler->load_prediction(app_id, description);
  if (are_features_enabled())
  {
    cluster = fs_handler->load_cluster(app_id, description);
  }

  if (is_prediction_valid())
  {
    logger->info(LOG_HEADER, "recovered predictions from storage.");
    set_state(InternalStatus::WITH_PREDICTION);
    return;
  }

  if (are_features_enabled() ? (is_cluster_valid() && are_models_valid()) : are_models_valid())
  {
    logger->info(LOG_HEADER, "recovered models", (are_features_enabled() ? " and clusters" : ""), " from storage.");
    set_state(InternalStatus::WITH_MODEL | InternalStatus::WITH_CLUSTER);

    logger->info(LOG_HEADER, "starting the prediction phase.");
    pid_t prediction_pid_t = start_prediction();
    Launcher::wait(prediction_pid_t);
    prediction = fs_handler->load_prediction(app_id, description);

    if (is_prediction_valid())
    {
      logger->info(LOG_HEADER, "the application knowledge has been retrieved.");
      set_state(InternalStatus::WITH_PREDICTION);
      return;
    }
    logger->info(LOG_HEADER, "the application knowledge was empty.");
  }

  if (is_doe_valid())
  {
    logger->info(LOG_HEADER, "recovered doe configurations from storage. Restaring the Design Space Exploration.");
    set_state(InternalStatus::EXPLORING | InternalStatus::WITH_DOE);
    return;
  }

  logger->info(LOG_HEADER, "starting the DOE building phase.");
  pid_t doe_pid = start_doe();
  Launcher::wait(doe_pid);
  doe = fs_handler->load_doe(app_id, description);

  if (!is_doe_valid())
  {
    logger->warning(LOG_HEADER, "the DOE plugin couldn't produce anything. Unstable.");
    set_state(InternalStatus::UNDEFINED);
    return;
  }

  logger->info(LOG_HEADER, "creating the observations container into the storage.");
  fs_handler->create_observation_table(app_id, description);

  logger->info(LOG_HEADER, "starting the Design Space Exploration.");
  set_state(InternalStatus::WITH_DOE | InternalStatus::EXPLORING);
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

pid_t RemoteApplicationHandler::start_doe()
{
  set_state(InternalStatus::BUILDING_DOE);

  logger->info(LOG_HEADER, "creating the doe plugin configuration file.");
  PluginConfiguration doe_config("plugin_config.env", app_id);
  fs_handler->create_env_configuration<PluginType::DOE>(doe_config);

  logger->info(LOG_HEADER, "starting the DOE generation process.");
  return doe_launcher->launch(doe_config);
}

std::vector<pid_t> RemoteApplicationHandler::start_modeling()
{
  set_state(InternalStatus::BUILDING_MODEL);

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
  set_state(InternalStatus::BUILDING_CLUSTER, false);

  logger->info(LOG_HEADER, "creating the cluster plugin configuration file");
  PluginConfiguration cluster_config("plugin_config.env", app_id);
  fs_handler->create_env_configuration<PluginType::Cluster>(cluster_config);

  // call the cluster plugin
  logger->info(LOG_HEADER, "starting the cluster generation process.");
  return cluster_launcher->launch(cluster_config);
}

pid_t RemoteApplicationHandler::start_prediction()
{
  set_state(InternalStatus::BUILDING_PREDICTION);

  logger->info(LOG_HEADER, "creating the prediction plugin configuration file");
  PluginConfiguration prediction_config("plugin_config.env", app_id);
  fs_handler->create_env_configuration<PluginType::Prediction>(prediction_config);

  // call the cluster plugin and wait for its completion
  logger->info(LOG_HEADER, "starting the application knowledge generation process.");
  return prediction_launcher->launch(prediction_config);
}

bool RemoteApplicationHandler::parse_informations(const std::string &info, margot::heel::block_model& description)
{
  try
  {
    margot::heel::application_model app_description;
    pt::ptree app_description_node;
    std::stringstream info_stream(info);
    pt::read_json(info_stream, app_description_node);
    margot::heel::parse(app_description, app_description_node);

    // store informations into the relative block description object
    auto block_itr = std::find_if(app_description.blocks.begin(), app_description.blocks.end(), [&](const auto &block) {
      return (app_description.name == app_id.app_name && app_description.version == app_id.version && block.name == app_id.block_name);
    });
    if (block_itr != app_description.blocks.end())
    {
      description = *block_itr;
      const_cast<int &>(num_configurations_per_iteration) = std::stoi(description.agora.number_configurations_per_iteration);
      set_state(InternalStatus::WITH_INFORMATION);
      return true;
    }
    return false;
  } catch (const std::exception &e)
  {
    logger->warning(LOG_HEADER, "there was a problem parsing the information -> ", e.what());
    return false;
  }
}

bool RemoteApplicationHandler::parse_observation(const std::string &observation_values, margot::heel::block_model &op_description)
{
  try
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
  } catch (const std::exception &e)
  {
    logger->warning(LOG_HEADER, "there was a problem parsing the observation -> ", e.what());
    return false;
  }
}

std::string RemoteApplicationHandler::configuration_to_json(const configuration_t &configuration) const
{
  pt::ptree op;
  pt::ptree knobs;
  for (const auto &knob : description.knobs)
  {
    knobs.put(knob.name, (knob.type == "string") ? ("\"" + configuration.at(knob.name) + "\"") : configuration.at(knob.name));
  }
  op.add_child("knobs", knobs);

  if (are_features_enabled())
  {
    pt::ptree features;
    for (const auto &feature : description.features.fields)
    {
      features.put(feature.name, 9999);
    }
    op.add_child("features", features);
  }

  pt::ptree metrics;
  for (const auto &metric : description.metrics)
  {
    if (metric.distribution)
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
  std::regex reg(R"r("(\d+\.?\d*)"|\n)r");
  return std::regex_replace(ss.str(), reg, "$1");
}

std::string RemoteApplicationHandler::prediction_to_json(const prediction_model &prediction) const
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
  std::regex reg(R"r("(\d+\.?\d*)"|\n)r");
  return std::regex_replace(ss.str(), reg, "$1");
}
