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

#ifndef MARGOT_AGORA_FS_OBSERVATION_HPP
#define MARGOT_AGORA_FS_OBSERVATION_HPP

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include <heel/model_block.hpp>

#include "agora/fs_configuration.hpp"
#include "agora/logger.hpp"

namespace agora {

class FsObservation {

public:
  static std::unique_ptr<FsObservation> get_instance(const FsConfiguration& configuration);

  virtual void create_observation_table(const application_id &app_id, const margot::heel::block_model &description) = 0;
  virtual void insert_observation_entry(const application_id &app_id, const std::string &client_id,
                                        const long duration_sec, const long duration_ns,
                                        const margot::heel::operating_point_model &operating_point) = 0;

  virtual std::string get_observation_name(const application_id &app_id) const = 0;

  virtual void erase(const application_id &app_id) = 0;

  virtual std::string get_type() const = 0;

  virtual ~FsObservation() {}

protected:
  FsObservation(const FsConfiguration& configuration);
  FsConfiguration configuration;

  std::shared_ptr<Logger> logger;
};

} // namespace agora

#endif // FS_OBSERVATION_HPP
