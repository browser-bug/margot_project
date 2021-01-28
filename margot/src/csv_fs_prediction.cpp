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
#include "agora/csv/csv_fs_prediction.hpp"
#include "agora/csv/csv.hpp"

namespace agora {

namespace fs = std::filesystem;

CsvPredictionStorage::CsvPredictionStorage(const FsConfiguration& configuation)
    : FsPrediction(configuation), csv_separator(configuation.csv_separator)
{
  prediction_dir = configuration.csv_storage_root_path / "predictions";
  fs::create_directories(prediction_dir);

  // add formatting settings to the csv parser
  format.delimiter(csv_separator);
}

void CsvPredictionStorage::store_prediction(const application_id &app_id, const margot::heel::block_model &description, const prediction_model &prediction)
{
  fs::create_directories(prediction_dir / app_id.path());

  // open the file for the prediction
  std::ofstream out;
  out.open(get_prediction_name(app_id), std::ios::out | std::ios::trunc);

  // write the common part of the header
  out << "pred_id";

  // write the header
  for (const auto &knob : description.knobs)
  {
    out << ',' << knob.name;
  }

  for (const auto &feature : description.features.fields)
  {
    out << ',' << feature.name;
  }

  for (const auto &metric : description.metrics)
  {
    out << ',' << metric.name + "_avg" << ',' << metric.name + "_std";
  }

  out << "\n";

  // loop over the entries of the prediction
  for (const auto &configuration : prediction.configurations)
  {
    std::string pred_id = configuration.first;
    out << pred_id;

    // write the configuration line
    for (const auto &knob : description.knobs)
    {
      out << ',' << configuration.second.at(knob.name);
    }

    // write the features line (if any)
    if(!description.features.fields.empty())
    {
      auto features_values = prediction.features.at(pred_id);

      for (const auto &feature : description.features.fields)
      {
        out << ',' << features_values.at(feature.name);
      }
    }

    // write the predicted values
    auto metric_values = prediction.predicted_results.at(pred_id);

    for (const auto &metric : description.metrics)
    {
      out << ',' << metric_values.at(metric.name)._avg << ',' << metric_values.at(metric.name)._std;
    }

    out << "\n";
  }
}

prediction_model CsvPredictionStorage::load_prediction(const application_id &app_id, const margot::heel::block_model &description)
{
  // declare the output prediction
  prediction_model output_prediction;

  try
  {
    // open the table of the prediction
    csv::CSVReader prediction_parser(get_prediction_name(app_id), format);

    // loop over the lines of the prediction
    for (auto &row : prediction_parser)
    {
      configuration_model config;
      features_model features;
      result_model output;

      for (const auto &knob : description.knobs)
      {
        config.insert_or_assign(knob.name, row[knob.name].get());
      }

      for (const auto &feature : description.features.fields)
      {
        features.insert_or_assign(feature.name, row[feature.name].get());
      }

      for (const auto &metric : description.metrics)
      {
        metric_value_model value = {row[metric.name + "_avg"].get(), row[metric.name + "_std"].get()};
        output.insert_or_assign(metric.name, value);
      }

      // add the prediction output
      output_prediction.add_result(row["pred_id"].get(), config, features, output);
    }
  } catch (const std::exception &e)
  {
    logger->warning("Csv prediction: error [", e.what(), "]. Returning an empty prediction.");
  }

  // return the model
  return output_prediction;
}

void CsvPredictionStorage::erase(const application_id &app_id)
{
  std::error_code ec;
  auto path = prediction_dir / app_id.path();
  fs::remove_all(path, ec);
  if (ec.value() != 0)
    logger->warning("Csv manager: unable to remove \"", path.string(), "\", err: ", ec.message());
}

} // namespace agora
