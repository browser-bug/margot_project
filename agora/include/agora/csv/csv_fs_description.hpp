/* Agora library
 * Copyright (C) 2021 Bernardo Menicagli
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef CSV_FS_DESCRIPTION_HPP
#define CSV_FS_DESCRIPTION_HPP

#include <filesystem>
#include <string>

#include "agora/fs_description.hpp"
#include "agora/logger.hpp"
#include "csv.hpp"

namespace agora {

class CsvDescriptionStorage : public FsDescription {

public:
  CsvDescriptionStorage(const FsConfiguration& configuration);

  ~CsvDescriptionStorage() = default;

  void store_description(const application_id &app_id, const margot::heel::block_model &description) override;
  margot::heel::block_model load_description(const application_id &application_id) override;

  std::string get_knobs_name(const application_id &app_id) const override
  {
    std::filesystem::path p = description_dir / app_id.path() / "knobs.csv";
    return p.string();
  }
  std::string get_features_name(const application_id &app_id) const override
  {
    std::filesystem::path p = description_dir / app_id.path() / "features.csv";
    return p.string();
  }
  std::string get_metrics_name(const application_id &app_id) const override
  {
    std::filesystem::path p = description_dir / app_id.path() / "metrics.csv";
    return p.string();
  }
  std::string get_properties_name(const application_id &app_id) const override
  {
    std::filesystem::path p = description_dir / app_id.path() / "agora_properties.csv";
    return p.string();
  }
  std::string get_doe_parameters_name(const application_id &app_id) const override
  {
    std::filesystem::path p = description_dir / app_id.path() / "doe_parameters.csv";
    return p.string();
  }
  std::string get_model_parameters_name(const application_id &app_id,
                                        const std::string &metric_name) const override
  {
    std::filesystem::path p = description_dir / app_id.path() / std::string(metric_name + "_model_parameters.csv");
    return p.string();
  }
  std::string get_clustering_parameters_name(const application_id &app_id) const override
  {
    std::filesystem::path p = description_dir / app_id.path() / "clustering_parameters.csv";
    return p.string();
  }

  void erase(const application_id &app_id) override;

  // the followings get the relative path for each specific table

  std::string get_type() const override { return "csv"; }

private:
  // this path will contain all the stored information
  std::filesystem::path description_dir;

  // configuration variables, for handling csv parsing
  const char csv_separator;
  csv::CSVFormat format;
};

} // namespace agora

#endif // CSV_FS_DESCRIPTION_HPP
