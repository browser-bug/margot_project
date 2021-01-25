#ifndef MARGOT_AGORA_FS_DESCRIPTION_HPP
#define MARGOT_AGORA_FS_DESCRIPTION_HPP

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include <heel/model_block.hpp>

#include "agora/fs_configuration.hpp"
#include "agora/logger.hpp"

namespace agora {

class FsDescription {

public:
  static std::unique_ptr<FsDescription> get_instance(const FsConfiguration& configuration);

  virtual void store_description(const application_id &app_id, const margot::heel::block_model &description) = 0;
  virtual margot::heel::block_model load_description(const application_id &application_id) = 0;

  virtual std::string get_knobs_name(const application_id &app_id) const = 0;
  virtual std::string get_features_name(const application_id &app_id) const = 0;
  virtual std::string get_metrics_name(const application_id &app_id) const = 0;
  virtual std::string get_properties_name(const application_id &app_id) const = 0;
  virtual std::string get_doe_parameters_name(const application_id &app_id) const = 0;
  virtual std::string get_model_parameters_name(const application_id &app_id, const std::string &metric_name) const = 0;
  virtual std::string get_clustering_parameters_name(const application_id &app_id) const = 0;

  virtual void erase(const application_id &app_id) = 0;

  virtual std::string get_type() const = 0;

  virtual ~FsDescription() {}

protected:
  FsDescription(const FsConfiguration& configuration);
  FsConfiguration configuration;

  std::shared_ptr<Logger> logger;
};

} // namespace agora

#endif // FS_DESCRIPTION_HPP
