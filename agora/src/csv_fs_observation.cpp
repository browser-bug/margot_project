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
#include "agora/csv/csv_fs_observation.hpp"
#include "agora/csv/csv.hpp"

namespace agora {

namespace fs = std::filesystem;

CsvObservationStorage::CsvObservationStorage(const FsConfiguration& configuation)
    : FsObservation(configuation), csv_separator(configuation.csv_separator)
{
  observation_dir = configuration.csv_storage_root_path / "observations";
  fs::create_directories(observation_dir);

  // add formatting settings to the csv parser
  format.delimiter(csv_separator);
}

void CsvObservationStorage::create_observation_table(const application_id &app_id, const margot::heel::block_model &description)
{
  fs::create_directories(observation_dir / app_id.path());

  // open the file for the obseration table
  std::ofstream out;
  out.open(get_observation_name(app_id), std::ios::out | std::ios::trunc);

  // write the common part of the header
  out << "sec,nanosec,client_id";

  // write the software knobs
  for (const auto &knob : description.knobs)
  {
    out << ',' << knob.name;
  }

  // write the input features
  for (const auto &feature : description.features.fields)
  {
    out << ',' << feature.name;
  }

  // write the metrics
  for (const auto &metric : description.metrics)
  {
    out << ',' << metric.name;
  }

  out << "\n";
}

void CsvObservationStorage::insert_observation_entry(const application_id &app_id, const std::string &client_id,
                                                     const long duration_sec, const long duration_ns,
                                                     const margot::heel::operating_point_model &operating_point)
{
  // open the file for the observation table and here we're just appending to avoid the annoying csv entire rewrite
  std::ofstream out;
  out.open(get_observation_name(app_id), std::ios::out | std::ios::app);

  out << duration_sec << ',' << duration_ns << ',' << client_id;

  for(const auto &knob : operating_point.knobs)
  {
    out << ',' << knob.mean;
  }

  for(const auto &feature : operating_point.features)
  {
    out << ',' << feature.mean;
  }

  for(const auto &metric : operating_point.metrics)
  {
    out << ',' << metric.mean;
  }

  out << "\n";
}

// declare a function to safely remove a file
void CsvObservationStorage::safe_rm(const fs::path &file_path)
{
  std::error_code ec;
  fs::remove(file_path, ec);
  if (ec.value() != 0)
    logger->warning("Csv manager: unable to remove the file \"", file_path.string(), "\", err: ", ec.message());
};

void CsvObservationStorage::erase(const application_id &app_id)
{
  safe_rm(get_observation_name(app_id));
}

} // namespace agora
