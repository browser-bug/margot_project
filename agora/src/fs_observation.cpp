#include "agora/fs_observation.hpp"
#include "agora/csv/csv_fs_observation.hpp"
#include "agora/application_manager.hpp"

using namespace agora;

FsObservation::FsObservation(const FsConfiguration& configuration) : configuration(configuration) {
  ApplicationManager& am = ApplicationManager::get_instance();
  logger = am.get_logger();
}

std::unique_ptr<FsObservation> FsObservation::get_instance(const FsConfiguration &configuration)
{
  std::unique_ptr<FsObservation> fs_observation;

  switch (configuration.observation_type)
  {
  case StorageType::CSV:
    fs_observation = std::unique_ptr<FsObservation>(new CsvObservationStorage(configuration));
    break;
  default:
    fs_observation = std::unique_ptr<FsObservation>(new CsvObservationStorage(configuration));
  }

  return fs_observation;
}
