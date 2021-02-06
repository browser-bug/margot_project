#include "agora/fs_description.hpp"
#include "agora/csv/csv_fs_description.hpp"
#include "agora/application_manager.hpp"

using namespace agora;

FsDescription::FsDescription(const FsConfiguration &configuration) : configuration(configuration)
{
  ApplicationManager &am = ApplicationManager::get_instance();
  logger = am.get_logger();
}

std::unique_ptr<FsDescription> FsDescription::get_instance(const FsConfiguration &configuration)
{
  std::unique_ptr<FsDescription> fs_description;

  switch (configuration.description_type)
  {
  case StorageType::CSV:
    fs_description = std::unique_ptr<FsDescription>(new CsvDescriptionStorage(configuration));
    break;
  default:
    fs_description = std::unique_ptr<FsDescription>(new CsvDescriptionStorage(configuration));
  }

  return fs_description;
}
