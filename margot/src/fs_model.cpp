#include "agora/application_manager.hpp"
#include "agora/fs_model.hpp"

namespace agora {

  FsModel::FsModel(const FsConfiguration &configuration) : configuration(configuration), model_dir(configuration.model_storage_root_path)
  {
    ApplicationManager &am = ApplicationManager::get_instance();
    logger = am.get_logger();
  }

  void FsModel::erase(const application_id &app_id) { safe_rm(get_models_path(app_id)); }

  void FsModel::safe_rm(const std::filesystem::path &file_path)
  {
    std::error_code ec;
    std::filesystem::remove(file_path, ec);
    if (ec.value() != 0)
      logger->warning("Csv manager: unable to remove the file \"", file_path.string(), "\", err: ", ec.message());
  }
}
