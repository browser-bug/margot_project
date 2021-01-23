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

  void safe_rm(const std::filesystem::path &file_path);
};

} // namespace agora

#endif // FS_MODEL_HPP
