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

#ifndef MARGOT_AGORA_FS_CLUSTER_HPP
#define MARGOT_AGORA_FS_CLUSTER_HPP

#include <string>
#include <vector>

#include <heel/model_block.hpp>

#include "agora/fs_configuration.hpp"
#include "agora/model_cluster.hpp"

namespace agora {

class FsCluster {

public:
  static std::unique_ptr<FsCluster> get_instance(const FsConfiguration& configuration);

  virtual void store_cluster(const application_id &app_id, const margot::heel::block_model &description, const cluster_model &cluster) = 0;
  virtual cluster_model load_cluster(const application_id &app_id, const margot::heel::block_model &description) = 0;

  virtual std::string get_cluster_name(const application_id &app_id) const = 0;

  virtual void erase(const application_id &app_id) = 0;

  virtual std::string get_type() const = 0;

  virtual ~FsCluster() {}

protected:
  FsCluster(const FsConfiguration& configuration);
  FsConfiguration configuration;

  std::shared_ptr<Logger> logger;
};

} // namespace agora

#endif // FS_CLUSTER_HPP
