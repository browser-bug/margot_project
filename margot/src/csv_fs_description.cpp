#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "agora/agora_properties.hpp"
#include "agora/csv/csv.hpp"
#include "agora/csv/csv_fs_description.hpp"

namespace agora {

namespace fs = std::filesystem;

CsvDescriptionStorage::CsvDescriptionStorage(const FsConfiguration &configuation)
    : FsDescription(configuation), csv_separator(configuation.csv_separator)
{
  description_dir = configuation.csv_storage_root_path / "descriptions";
  fs::create_directories(description_dir);

  // add formatting settings to the csv parser
  format.delimiter(csv_separator);
}

void CsvDescriptionStorage::store_description(const application_id &app_id, const margot::heel::block_model &description)
{
  fs::create_directories(description_dir / app_id.path());

  std::ofstream out;

  // storing the software knobs
  out.open(get_knobs_name(app_id), std::ios::out | std::ios::trunc);

  if (out.is_open())
  {
    // print the table header
    out << "name,type,values\n";

    // loop over the software knobs and print them
    for (const auto &knob : description.knobs)
    {
      out << knob.name << ',' << knob.type;

      out << ',' << knob.values.front();
      for (auto knob_val = knob.values.begin() + 1; knob_val != knob.values.end(); ++knob_val)
        out << ';' << *knob_val;

      out << "\n";
    }
  } else
  {
    logger->warning("Csv manager: unable to open/create file \"", get_knobs_name(app_id), "\"");
    throw std::runtime_error("Csv manager: unable to open/create file \"" + get_knobs_name(app_id) + "\"");
  }

  out.close();

  // storing the metrics
  out.open(get_metrics_name(app_id), std::ios::out | std::ios::trunc);

  if (out.is_open())
  {
    // print the table header
    out << "name,type,prediction\n";

    // loop over the metrics and print them
    for (const auto &metric : description.metrics)
    {
      out << metric.name << ',' << metric.type << ',' << metric.prediction_plugin << "\n";
    }
  } else
  {
    logger->warning("Csv manager: unable to open/create file \"", get_metrics_name(app_id), "\"");
    throw std::runtime_error("Csv manager: unable to open/create file \"" + get_metrics_name(app_id) + "\"");
  }

  out.close();

  // storing the input features
  out.open(get_features_name(app_id), std::ios::out | std::ios::trunc);

  if (out.is_open())
  {
    // print the table header
    out << "name,type\n";

    // loop over the features and print them
    for (const auto &feature : description.features.fields)
    {
      out << feature.name << ',' << feature.type << "\n";
    }
  } else
  {
    logger->warning("Csv manager: unable to open/create file \"", get_features_name(app_id), "\"");
    throw std::runtime_error("Csv manager: unable to open/create file \"" + get_features_name(app_id) + "\"");
  }

  out.close();

  // storing the agora informations
  out.open(get_properties_name(app_id), std::ios::out | std::ios::trunc);

  if (out.is_open())
  {
    // print the table header
    out << "property_name,value\n";

    // print the actual properties
    out << "doe_plugin," << description.agora.doe_plugin << "\n";
    out << "clustering_plugin," << description.agora.clustering_plugin << "\n";
    out << "number_configurations_per_iteration," << description.agora.number_configurations_per_iteration << "\n";
    out << "number_observations_per_configuration," << description.agora.number_observations_per_configuration << "\n";
    out << "max_number_iteration," << description.agora.max_number_iteration << "\n";
  } else
  {
    logger->warning("Csv manager: unable to open/create file \"", get_properties_name(app_id), "\"");
    throw std::runtime_error("Csv manager: unable to open/create file \"" + get_properties_name(app_id) + "\"");
  }

  out.close();

  // storing model parameters for each metric
  for (const auto &metric : description.metrics)
  {
    if (!metric.prediction_parameters.empty())
    {
      out.open(get_model_parameters_name(app_id, metric.name), std::ios::out | std::ios::trunc);
      if (out.is_open())
      {
        // print the table header
        out << "parameter_name,value\n";

        // print the values
        for (const auto &param : metric.prediction_parameters)
          out << param.key << ',' << param.value << "\n";
      } else
      {
        logger->warning("Csv manager: unable to open/create file \"", get_model_parameters_name(app_id, metric.name), "\"");
        throw std::runtime_error("Csv manager: unable to open/create file \"" + get_model_parameters_name(app_id, metric.name) + "\"");
      }
      out.close();
    }
  }

  // storing doe parameters
  out.open(get_doe_parameters_name(app_id), std::ios::out | std::ios::trunc);

  if (out.is_open())
  {
    // print the table header
    out << "parameter_name,value\n";

    // print the values
    for (const auto &param : description.agora.doe_parameters)
      out << param.key << ',' << param.value << "\n";
  } else
  {
    logger->warning("Csv manager: unable to open/create file \"", get_doe_parameters_name(app_id), "\"");
    throw std::runtime_error("Csv manager: unable to open/create file \"" + get_doe_parameters_name(app_id) + "\"");
  }

  out.close();

  // storing clustering parameters
  out.open(get_clustering_parameters_name(app_id), std::ios::out | std::ios::trunc);

  if (out.is_open())
  {
    // print the table header
    out << "parameter_name,value\n";

    // print the values
    for (const auto &param : description.agora.clustering_parameters)
      out << param.key << ',' << param.value << "\n";
  } else
  {
    logger->warning("Csv manager: unable to open/create file \"", get_clustering_parameters_name(app_id), "\"");
    throw std::runtime_error("Csv manager: unable to open/create file \"" + get_clustering_parameters_name(app_id) + "\"");
  }

  out.close();
}

margot::heel::block_model CsvDescriptionStorage::load_description(const application_id &app_id)
{
  // initialize the object
  margot::heel::block_model description;
  description.name = app_id.block_name;

  // open the table of the knobs
  csv::CSVReader knob_parser(get_knobs_name(app_id), format);

  // loop over the lines of the knobs
  for (auto &row : knob_parser)
  {
    // get the knob name and type, prepare the stream for values
    const std::string knob_name = row["name"].get();
    const std::string knob_type = row["type"].get();
    std::stringstream knob_values_stream(row["values"].get());
    std::vector<std::string> knob_values;
    std::string knob_value;

    // parse all the possible values for the stream
    while (std::getline(knob_values_stream, knob_value, ';'))
    {
      knob_values.push_back(knob_value);
    }

    // emplace the knob
    margot::heel::knob_model knob = {knob_name, knob_type, knob_values};
    description.knobs.push_back(knob);
  }

  // open the table of the features
  csv::CSVReader feature_parser(get_features_name(app_id), format);

  // loop over the lines of the features
  for (auto &row : feature_parser)
  {
    // get the feature name and type, prepare the stream for values
    const std::string feature_name = row["name"].get();
    const std::string feature_type = row["type"].get();

    // emplace the feature
    margot::heel::feature_model feature = {feature_name, feature_type};
    description.features.fields.push_back(feature);
  }

  // open the table of the metrics
  csv::CSVReader metric_parser(get_metrics_name(app_id), format);

  // loop over the lines of the knobs
  for (auto &row : metric_parser)
  {
    // get the metric name, type, prediction method and prediction parameters
    const std::string metric_name = row["name"].get();
    const std::string metric_type = row["type"].get();
    const std::string metric_predictor = row["prediction"].get();

    // emplace the metric
    margot::heel::metric_model metric;
    metric.name = metric_name;
    metric.type = metric_type;
    metric.prediction_plugin = metric_predictor;
    description.metrics.push_back(metric);
  }

  // open the table of the doe info
  csv::CSVReader properties_parser(get_properties_name(app_id), format);

  // loop over the lines of the properties information
  for (auto &row : properties_parser)
  {
    // read the property
    const std::string property_name = row["property_name"].get();
    const std::string property_value = row["value"].get();

    // assign the correct value
    switch (resolve_setting_type(property_name))
    {
    case AgoraSettingType::Number_Config_Per_Iter:
      description.agora.number_configurations_per_iteration = property_value;
      break;
    case AgoraSettingType::Number_Obs_Per_Config:
      description.agora.number_observations_per_configuration = property_value;
      break;
    case AgoraSettingType::Max_Number_Iter:
      description.agora.max_number_iteration = property_value;
      break;
    case AgoraSettingType::Doe_Plugin:
      description.agora.doe_plugin = property_value;
      break;
    case AgoraSettingType::Clustering_Plugin:
      description.agora.clustering_plugin = property_value;
      break;
    // case AgoraProperties::Storage_Type:
    // description.agora.storage_type = property_value;
    // break;
    // case AgoraProperties::Storage_Address:
    // description.agora.storage_address = property_value;
    // break;
    // case AgoraProperties::Storage_Username:
    // description.agora.storage_username = property_value;
    // break;
    // case AgoraProperties::Storage_Password:
    // description.agora.storage_password = property_value;
    // break;
    default:
      logger->warning("Csv manager: unknown agora property \"", property_name, "\" with value \"", property_value, "\"");
    }
  }

  // open the table of the model parameters for each metric
  for (auto &metric : description.metrics)
  {
    csv::CSVReader model_parameters_parser(get_model_parameters_name(app_id, metric.name), format);

    for (auto &row : model_parameters_parser)
    {
      // read the parameter
      margot::heel::pair_property param;
      param.key = row["parameter_name"].get();
      param.value = row["value"].get();

      metric.prediction_parameters.push_back(param);
    }
  }

  // open the table of the doe parameters
  csv::CSVReader doe_parameters_parser(get_doe_parameters_name(app_id), format);

  for (auto &row : doe_parameters_parser)
  {
    // read the parameter
    margot::heel::pair_property param;
    param.key = row["parameter_name"].get();
    param.value = row["value"].get();

    description.agora.doe_parameters.push_back(param);
  }

  // open the table of the clustering parameters
  csv::CSVReader clustering_parameters_parser(get_clustering_parameters_name(app_id), format);

  for (auto &row : clustering_parameters_parser)
  {
    // read the parameter
    margot::heel::pair_property param;
    param.key = row["parameter_name"].get();
    param.value = row["value"].get();

    description.agora.clustering_parameters.push_back(param);
  }

  return description;
}

// declare a function to safely remove a file
void CsvDescriptionStorage::safe_rm(const fs::path &file_path)
{
  std::error_code ec;
  fs::remove(file_path, ec);
  if (ec.value() != 0)
    logger->warning("Csv manager: unable to remove the file \"", file_path.string(), "\", err: ", ec.message());
};

void CsvDescriptionStorage::erase(const application_id &app_id, const margot::heel::block_model &description)
{
  safe_rm(get_knobs_name(app_id));
  safe_rm(get_features_name(app_id));
  safe_rm(get_metrics_name(app_id));
  safe_rm(get_properties_name(app_id));
  safe_rm(get_doe_parameters_name(app_id));
  for (auto &metric : description.metrics)
    safe_rm(get_model_parameters_name(app_id, metric.name));
  safe_rm(get_clustering_parameters_name(app_id));
}

} // namespace agora
