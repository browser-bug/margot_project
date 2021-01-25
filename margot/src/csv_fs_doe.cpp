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
#include "agora/csv/csv_fs_doe.hpp"
#include "agora/csv/csv.hpp"

namespace agora {

namespace fs = std::filesystem;

CsvDoeStorage::CsvDoeStorage(const FsConfiguration& configuation)
    : FsDoe(configuation), csv_separator(configuation.csv_separator)
{
  doe_dir = configuration.csv_storage_root_path / "configurations";
  fs::create_directories(doe_dir);

  // add formatting settings to the csv parser
  format.delimiter(csv_separator);
}

void CsvDoeStorage::store_doe(const application_id &app_id, const margot::heel::block_model &description, const doe_model &doe)
{
  fs::create_directories(doe_dir / app_id.path());

  // open the file for the doe
  std::ofstream out;
  out.open(get_doe_name(app_id), std::ios::out | std::ios::trunc);

  // write the header
  out << "config_id,counter";

  for (const auto &knob : description.knobs)
  {
    out << ',' << knob.name;
  }

  out << "\n";

  // write the all the required explorations
  for (const auto &configuration : doe.required_explorations)
  {
    const std::string config_id = configuration.first;
    const int num_explorations = doe.number_of_explorations.at(config_id);
    out << config_id << ',' << num_explorations;
    for(const auto& knob : description.knobs)
      out << ',' << configuration.second.at(knob.name);
    out << "\n";
  }
}

doe_model CsvDoeStorage::load_doe(const application_id &app_id, const margot::heel::block_model &description)
{
  // declare the output doe
  doe_model output_doe;

  // open the table of the doe
  csv::CSVReader doe_parser(get_doe_name(app_id), format);

  // go through each line of the model
  for (auto &row : doe_parser)
  {
    const std::string id = row["config_id"].get();
    const int counter = std::stoi(row["counter"].get());

    // insert the configuration only if still required
    if (counter > 0)
    {
      configuration_model config;
      for(const auto& knob : description.knobs)
      {
        config.insert({knob.name, row[knob.name].get()});
      }

      if (output_doe.add_config(id, config, counter))
      {
        logger->warning("CSV Doe: the configuration with ID ", id, " was already present. Replacing it.");
      }
    }
  }

  return output_doe;
}

void CsvDoeStorage::update_doe(const application_id &app_id, const margot::heel::block_model &description, const std::string &config_id)
{
  // load the doe table
  doe_model output_doe = load_doe(app_id, description);

  // update the specific configuration
  output_doe.update_config(config_id);

  // rewrite the doe
  store_doe(app_id, description, output_doe);
}

void CsvDoeStorage::empty_doe_entries(const application_id &app_id, const margot::heel::block_model &description)
{
  // open the file for the doe
  std::ofstream out;
  out.open(get_doe_name(app_id), std::ios::out | std::ios::trunc);

  // write the header
  out << "config_id,counter";

  for (const auto &knob : description.knobs)
  {
    out << ',' << knob.name;
  }

  out << "\n";
}

void CsvDoeStorage::erase(const application_id& app_id)
{
  std::error_code ec;
  auto path = doe_dir / app_id.path();
  fs::remove_all(path, ec);
  if (ec.value() != 0)
    logger->warning("Csv manager: unable to remove \"", path.string(), "\", err: ", ec.message());
}

} // namespace agora
