#include "agora/fs_doe.hpp"
#include "agora/csv/csv_fs_doe.hpp"
#include "agora/application_manager.hpp"

using namespace agora;

FsDoe::FsDoe(const FsConfiguration& configuration) : configuration(configuration) {
  ApplicationManager& am = ApplicationManager::get_instance();
  logger = am.get_logger();
}

std::unique_ptr<FsDoe> FsDoe::get_instance(const FsConfiguration &configuration)
{
  std::unique_ptr<FsDoe> fs_doe;

  switch (configuration.doe_type)
  {
  case StorageType::CSV:
    fs_doe = std::unique_ptr<FsDoe>(new CsvDoeStorage(configuration));
    break;
  default:
    fs_doe = std::unique_ptr<FsDoe>(new CsvDoeStorage(configuration));
  }

  return fs_doe;
}
