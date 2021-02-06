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
