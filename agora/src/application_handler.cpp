#include <boost/property_tree/json_parser.hpp>
#include <heel/model_application.hpp>
#include <heel/parser_application.hpp>
#include <heel/parser_operating_point.hpp>

#include "agora/application_handler.hpp"
#include "agora/application_manager.hpp"

using namespace agora;
namespace pt = boost::property_tree;
namespace fs = std::filesystem;

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

void RemoteApplicationHandler::welcome_client(const client_id_t &cid, const std::string &info)
{
  // we lock earlier because we have to check all different possible states to get a clue of what's going on
  std::unique_lock<std::mutex> lock(app_mutex);

  // this is a new client so we register it
  active_clients.emplace(cid);

  // the handler has just been created, so this is the first client that has been received
  if (status == ApplicationStatus::CLUELESS)
  {
    logger->info(LOG_HEADER, "a new application has joined the pool. Retrieving informations.");

    logger->info(LOG_HEADER, "receiving and parsing informations on the application.");

    // parse the app informations
    margot::heel::application_model app_description;
    pt::ptree app_description_node;
    std::stringstream info_stream(info);
    pt::read_json(info_stream, app_description_node);
    margot::heel::parse(app_description, app_description_node);

    // store informations into the relative block description object
    for (auto &block : app_description.blocks)
    {
      if (app_description.name == app_id.app_name && block.name == app_id.block_name)
      {
        description = block;
        num_configurations_per_iteration = std::stoi(description.agora.number_configurations_per_iteration);
        break;
      }
    }

    // setup the plugin launchers with the information received
    doe_launcher = Launcher::get_instance(launcher_configuration, description.agora.doe_plugin);
    doe_launcher->initialize_workspace(app_id);

    if (!description.features.fields.empty())
    {
      cluster_launcher = Launcher::get_instance(launcher_configuration, description.agora.clustering_plugin);
      cluster_launcher->initialize_workspace(app_id);
    }

    // TODO: this is a value that needs to be specified on margot_heel configuration file, for now let's use a default
    prediction_launcher = Launcher::get_instance(launcher_configuration, "prediction");
    prediction_launcher->initialize_workspace(app_id);

    // adding a new launcher for every metric we have
    for (const auto &metric : description.metrics)
    {
      // we could have the same plugin launcher for two different metrics, so we have to check on that
      if (model_launchers.find(metric.name) != model_launchers.end())
        continue;
      auto model_launcher = Launcher::get_instance(launcher_configuration, metric.prediction_plugin);
      model_launcher->initialize_workspace(app_id);
      model_launchers.insert({metric.name, model_launcher});
    }

    logger->info(LOG_HEADER, "storing description informations.");
    fs_handler->store_description(app_id, description);

    logger->info(LOG_HEADER, "creating required containers into the storage");
    fs_handler->create_observation_table(app_id, description);

    // start the doe building phase
    status = ApplicationStatus::BUILDING_DOE;
    lock.unlock();

    // creating the doe plugin configuratin file
    logger->info(LOG_HEADER, "creating the doe plugin configuration file.");
    PluginConfiguration doe_config(app_id);
    fs_handler->create_env_configuration<PluginType::DOE>(doe_config);
    doe_launcher->set_plugin_configuration(doe_config);

    // call the doe plugin and wait for its completion
    logger->info(LOG_HEADER, "starting the DOE generation process.");
    doe_launcher->launch();
    doe_launcher->wait();

    // retrieve the doe informations produced
    doe = fs_handler->load_doe(app_id, description);

    // start the DSE phase
    lock.lock();

    if (doe.required_explorations.empty())
    {
      logger->warning(LOG_HEADER, "no configuration to explore.");
      return;
    }
    if (active_clients.empty())
    {
      logger->warning(LOG_HEADER, "no active clients to send configurations to.");
      return;
    }

    logger->info(LOG_HEADER, "starting the Design Space Exploration.");
    status = ApplicationStatus::EXPLORING;

    // send the available configurations to the active clients
    for (const auto &client_name : active_clients)
    {
      send_configuration(client_name);
    }

    return;
  }

  // we're building the doe so just wait for a configuration to explore
  if( status == ApplicationStatus::BUILDING_DOE)
  {
    logger->info(LOG_HEADER, "building a new DOE, wait for a configuration to explore.");
    return;
  }

  // we have already a configuration available and we can start exploring
  if (status == ApplicationStatus::EXPLORING)
  {
    logger->info(LOG_HEADER, "sending a new configuration to explore.");
    send_configuration(cid);
    return;
  }

  // we already have a prediction available and we can share to the new client the application knowledge
  if (status == ApplicationStatus::WITH_PREDICTION)
  {
    logger->info(LOG_HEADER, "sending the application knowledge to the new client.");
    send_prediction(cid);
    return;
  }
}

void RemoteApplicationHandler::bye_client(const client_id_t &cid)
{
  logger->info(LOG_HEADER, "terminating connection with client \"", cid, "\".");

  std::unique_lock<std::mutex> lock(app_mutex);

  active_clients.erase(cid);

  if (status == ApplicationStatus::EXPLORING)
  {
    logger->info(LOG_HEADER, "removing assigned configurations (if any) for client \"", cid, "\".");
    assigned_configurations.erase(cid);
  }

  // TODO: if it was the last client do things
}

void RemoteApplicationHandler::process_observation(const client_id_t &cid, const long duration_sec, const long duration_ns,
                                                   const std::string &observation_values)
{
  // parse the operating point data
  margot::heel::block_model op_description;
  op_description.name = description.name;
  op_description.knobs = description.knobs;
  op_description.features.fields = description.features.fields;
  op_description.metrics = description.metrics;

  pt::ptree op_node;
  std::stringstream op_stream(observation_values);
  pt::read_json(op_stream, op_node);
  margot::heel::parse_operating_points(op_description, op_node);

  // check if we actually got the OP
  if (op_description.ops.empty())
  {
    logger->warning(LOG_HEADER, "the operating point received failed to be parsed.");
    return;
  }

  std::unique_lock<std::mutex> lock(app_mutex);

  // if we're not exploring configurations, we're not interested in this message type
  if (status != ApplicationStatus::EXPLORING)
    return;

  fs_handler->insert_observation_entry(app_id, cid, duration_sec, duration_ns, op_description.ops.front());

  // TODO: check if the configuration received is matched with the configurations assigned to the client?

  // if we still have some explorations to do we can't continue yet
  if (!doe.required_explorations.empty() && num_configurations_sent_per_iteration <= num_configurations_per_iteration)
  {
    send_configuration(cid);
    return;
  }

  iteration_number++;
  num_configurations_sent_per_iteration = 0;
  status = ApplicationStatus::BUILDING_MODEL;
  //// TODO: this is something that will be issued in parallel with respect to the modeling phase, so in the future with the status bitmask
  /// we / will do something like this status = BUILDING_MODEL | BUILDING_CLUSTER
  // status = ApplicationStatus::BUILDING_CLUSTER;
  lock.unlock();

  logger->info(LOG_HEADER, "starting the model phase.");

  // creating the model plugin configuration file for each metric
  logger->info(LOG_HEADER, "creating the model plugin configuration file for each metric.");
  std::unordered_map<std::string, PluginConfiguration> model_configs;
  for (const auto &metric : description.metrics)
  {
    PluginConfiguration model_config(app_id, metric.name, iteration_number);
    fs_handler->create_env_configuration<PluginType::Model>(model_config);
    model_configs.insert({metric.name, model_config});
  }

  // call the model plugin for each metric
  for (const auto &config : model_configs)
  {
    auto model_launcher = model_launchers.at(config.first);
    model_launcher->set_plugin_configuration(config.second);

    logger->info(LOG_HEADER, "starting the model generation process for the metric [", config.first, "].");
    model_launcher->launch();
  }

  if (!description.features.fields.empty())
  {
    logger->info(LOG_HEADER, "starting the clustering phase.");

    logger->info(LOG_HEADER, "creating the cluster plugin configuration file");
    PluginConfiguration cluster_config(app_id);
    fs_handler->create_env_configuration<PluginType::Cluster>(cluster_config);
    cluster_launcher->set_plugin_configuration(cluster_config);

    // call the cluster plugin
    logger->info(LOG_HEADER, "starting the cluster generation process.");
    cluster_launcher->launch();
    cluster_launcher->wait();

    // retrieve the cluster informaton
    cluster = fs_handler->load_cluster(app_id, description);
  }

  // wait for the models generation
  for (const auto &launcher : model_launchers)
    launcher.second->wait();

  lock.lock();

  // check if the models were valid for each metric
  bool are_models_valid = true;
  for (const auto &metric : description.metrics)
  {
    const auto model_path = fs::path(fs_handler->get_model_name(app_id, metric.name));
    if (!fs::exists(model_path))
    {
      are_models_valid = false;
      break;
    }
  }

  // if everything went fine, we can generate the application knowledge
  if (are_models_valid)
  {
    status = ApplicationStatus::WITH_MODEL;
    lock.unlock();

    // launch the prediction plugin
    logger->info(LOG_HEADER, "starting the prediction phase.");

    logger->info(LOG_HEADER, "creating the prediction plugin configuration file");
    PluginConfiguration prediction_config(app_id);
    fs_handler->create_env_configuration<PluginType::Prediction>(prediction_config);
    prediction_launcher->set_plugin_configuration(prediction_config);

    // call the cluster plugin and wait for its completion
    logger->info(LOG_HEADER, "starting the application knowledge generation process.");
    prediction_launcher->launch();
    prediction_launcher->wait();

    lock.lock();
    // retrieve the final predictions
    prediction = fs_handler->load_prediction(app_id, description);
    logger->info(LOG_HEADER, "the application knowledge has been retrieved.");
    status = ApplicationStatus::WITH_PREDICTION;

    // finally broadcast the application knowledge and exit
    for (auto &cid : active_clients)
      send_prediction(cid);
    return;
  }

  // some of the models are not valid so we need to collect more observations
  // TODO: for now we should never enter this stage since all the possible configurations are already stored inside doe
  if (doe.required_explorations.empty())
  {
    status = ApplicationStatus::BUILDING_DOE;
    lock.unlock();

    logger->info(LOG_HEADER, "no more configurations avilable.");

    // call the doe plugin and wait for its completion
    logger->info(LOG_HEADER, "starting the DOE generation process once again.");
    doe_launcher->launch();
    doe_launcher->wait();

    lock.lock();
    // retrieve the doe informations produced
    doe = fs_handler->load_doe(app_id, description);
  }

  // start the DSE phase
  if (doe.required_explorations.empty())
  {
    logger->warning(LOG_HEADER, "no configuration to explore.");
    return;
  }
  if (active_clients.empty())
  {
    logger->warning(LOG_HEADER, "no active clients.");
    return;
  }

  logger->info(LOG_HEADER, "starting the Design Space Exploration.");
  status = ApplicationStatus::EXPLORING;
  // send the available configurations to the active clients
  for (const auto &cid : active_clients)
  {
    send_configuration(cid);
  }
}

const std::string RemoteApplicationHandler::configuration_to_json(const configuration_model &configuration) const
{
  std::ostringstream json_string;

  json_string << "{\"" << description.name << "\": ["; // block
  json_string << "{";                                  // operating_point

  json_string << "\"knobs\": {"; // knobs
  for (const auto &knob : description.knobs)
    json_string << "\"" << knob.name << "\":" << configuration.at(knob.name) << ",";
  json_string.seekp(-1, json_string.cur);
  json_string << "}"; // knobs

  json_string << "}";  // operating_point
  json_string << "]}"; // block

  return json_string.str();
}

const std::string RemoteApplicationHandler::prediction_to_json(const prediction_model &prediction) const
{
  std::ostringstream json_string;

  json_string << "{\"" << description.name << "\": ["; // block

  // print all the operating points
  for (const auto &result : prediction.predicted_results)
  {
    const auto pred_id = result.first;

    json_string << "{"; // operating_point

    if (!description.features.fields.empty())
    {
      json_string << "\"features\":{";
      for (const auto &feature : description.features.fields)
      {
        json_string << "\"" << feature.name << "\":" << prediction.features.at(pred_id).at(feature.name) << ",";
      }
      json_string.seekp(-1, json_string.cur);
      json_string << "},";
    }

    json_string << "\"knobs\":{";
    for (const auto &knob : description.knobs)
    {
      json_string << "\"" << knob.name << "\":" << prediction.configurations.at(pred_id).at(knob.name) << ",";
    }
    json_string.seekp(-1, json_string.cur);
    json_string << "},";

    json_string << "\"metrics\":{";
    for (const auto &metric : description.metrics)
    {
      json_string << "\"" << metric.name << "\":";
      if (metric.distribution)
        json_string << "[" << prediction.predicted_results.at(pred_id).at(metric.name)._avg
                    << prediction.predicted_results.at(pred_id).at(metric.name)._std << "]";
      else
        json_string << prediction.predicted_results.at(pred_id).at(metric.name)._avg;
      json_string << ",";
    }
    json_string.seekp(-1, json_string.cur);
    json_string << "}";

    json_string << "},"; // operating_point
  }

  json_string.seekp(-1, json_string.cur);
  json_string << "]}"; // block

  return json_string.str();
}
