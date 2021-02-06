#include "agora/fs_model.hpp"
#include "agora/application_manager.hpp"

namespace agora {

namespace fs = std::filesystem;

FsModel::FsModel(const FsConfiguration &configuration) : configuration(configuration), model_dir(configuration.model_storage_root_path)
{
  ApplicationManager &am = ApplicationManager::get_instance();
  logger = am.get_logger();
}

void FsModel::erase(const application_id &app_id)
{
  std::error_code ec;
  auto path = model_dir / app_id.path();
  fs::remove_all(path, ec);
  if (ec.value() != 0)
    logger->warning("Model manager: unable to remove \"", path.string(), "\", err: ", ec.message());
}

} // namespace agora
