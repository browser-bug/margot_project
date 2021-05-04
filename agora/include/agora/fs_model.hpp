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

#ifndef FS_MODEL_HPP
#define FS_MODEL_HPP

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include <heel/model_block.hpp>

#include "agora/agora_properties.hpp"
#include "agora/fs_configuration.hpp"
#include "agora/logger.hpp"

namespace agora {

class FsModel {

public:
  static std::unique_ptr<FsModel> get_instance(const FsConfiguration &configuration)
  {
    return std::unique_ptr<FsModel>(new FsModel(configuration));
  }

  bool is_model_valid(const application_id &app_id, const std::string &metric_name) const
  {
    return std::filesystem::exists(get_model_name(app_id, metric_name));
  }

  std::string get_models_path(const application_id &app_id) const
  {
    std::filesystem::path p = model_dir / app_id.path();
    return p.string();
  }
  std::string get_model_name(const application_id &app_id, const std::string &metric_name) const
  {
    std::filesystem::path p = model_dir / app_id.path() / std::string(metric_name + "_model.data");
    return p.string();
  }

  void erase(const application_id &app_id);

private:
  FsModel(const FsConfiguration &configuration);

  FsConfiguration configuration;

  std::filesystem::path model_dir;

  std::shared_ptr<Logger> logger;
};

} // namespace agora

#endif // FS_MODEL_HPP
