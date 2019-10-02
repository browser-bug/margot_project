/* agora/csv_fs_implementation.hpp
 * Copyright (C) 2018 Davide Gadioli
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

#ifndef MARGOT_AGORA_CSV_FS_IMPLEMENTATION_HDR
#define MARGOT_AGORA_CSV_FS_IMPLEMENTATION_HDR

#include <string>

#include "agora/common_objects.hpp"
#include "agora/fs_handler.hpp"

namespace agora {

class CsvStorage : public FsHandler {
  // this path will contain all the stored information
  const std::string storage_main_folder;

  // configuration variables, for handling namespaces
  const char default_application_separator;
  const char table_application_separator;

 public:
  CsvStorage(const std::string& storage_root_path);
  ~CsvStorage(void) = default;

  void store_description(const application_description_t& description);
  application_description_t load_description(const std::string& application_name);

  void store_model(const application_description_t& description, const model_t& model);
  model_t load_model(const application_description_t& description);

  void store_doe(const application_description_t& description, const doe_t& doe);
  doe_t load_doe(const std::string& application_name);
  void update_doe(const application_description_t& description, const std::string& values);
  void empty_doe_entries(const application_description_t& description);

  void create_trace_table(const application_description_t& description);
  void insert_trace_entry(const application_description_t& description, const std::string& values);

  void erase(const std::string& application_name);

  bool support_concurrency(void) const { return false; }

  std::string get_type(void) const { return "CSV"; }
  std::string get_address(void) const { return storage_main_folder; }
  std::string get_username(void) const { return ""; }
  std::string get_password(void) const { return ""; }
  std::string get_observation_name(const std::string& application_name) const {
    std::string table_name = application_name;
    std::replace(table_name.begin(), table_name.end(), default_application_separator,
                 table_application_separator);
    return storage_main_folder + "/" + table_name + "_trace.csv";
  }
  std::string get_model_name(const std::string& application_name) const {
    std::string table_name = application_name;
    std::replace(table_name.begin(), table_name.end(), default_application_separator,
                 table_application_separator);
    return storage_main_folder + "/" + table_name + "_model.csv";
  }

  std::string get_knobs_name(const std::string& application_name) const {
    std::string table_name = application_name;
    std::replace(table_name.begin(), table_name.end(), default_application_separator,
                 table_application_separator);
    return storage_main_folder + "/" + table_name + "_knobs.csv";
  }
  std::string get_features_name(const std::string& application_name) const {
    std::string table_name = application_name;
    std::replace(table_name.begin(), table_name.end(), default_application_separator,
                 table_application_separator);
    return storage_main_folder + "/" + table_name + "_features.csv";
  }
  std::string get_metrics_name(const std::string& application_name) const {
    std::string table_name = application_name;
    std::replace(table_name.begin(), table_name.end(), default_application_separator,
                 table_application_separator);
    return storage_main_folder + "/" + table_name + "_metrics.csv";
  }
  std::string get_doe_name(const std::string& application_name) const {
    std::string table_name = application_name;
    std::replace(table_name.begin(), table_name.end(), default_application_separator,
                 table_application_separator);
    return storage_main_folder + "/" + table_name + "_doe.csv";
  }
  std::string get_doe_info_name(const std::string& application_name) const {
    std::string table_name = application_name;
    std::replace(table_name.begin(), table_name.end(), default_application_separator,
                 table_application_separator);
    return storage_main_folder + "/" + table_name + "_doe_info.csv";
  }
};

}  // namespace agora

#endif  // MARGOT_AGORA_CSV_FS_IMPLEMENTATION_HDR
