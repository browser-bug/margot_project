#ifndef MARGOT_AGORA_FS_DOE_HPP
#define MARGOT_AGORA_FS_DOE_HPP

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include <heel/model_block.hpp>

#include "agora/fs_configuration.hpp"
#include "agora/model_doe.hpp"

namespace agora {

class FsDoe {

public:
  static std::unique_ptr<FsDoe> get_instance(const FsConfiguration& configuration);

  virtual void store_doe(const application_id &app_id, const margot::heel::block_model &description, const doe_model &doe) = 0;
  virtual doe_model load_doe(const application_id &app_id, const margot::heel::block_model &description) = 0;
  virtual void update_doe(const application_id &app_id, const margot::heel::block_model &description, const std::string &config_id) = 0;
  virtual void empty_doe_entries(const application_id &app_id, const margot::heel::block_model &description) = 0;

  virtual std::string get_doe_name(const application_id &app_id) const = 0;
  virtual std::string get_total_configurations_name(const application_id &app_id) const = 0;

  virtual void erase(const application_id &app_id) = 0;

  virtual std::string get_type() const = 0;

  virtual ~FsDoe() {}

protected:
  FsDoe(const FsConfiguration& configuration);
  FsConfiguration configuration;

  std::shared_ptr<Logger> logger;
};

} // namespace agora

#endif // FS_DOE_HPP
