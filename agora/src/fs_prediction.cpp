#include "agora/fs_prediction.hpp"
#include "agora/csv/csv_fs_prediction.hpp"
#include "agora/application_manager.hpp"

using namespace agora;

FsPrediction::FsPrediction(const FsConfiguration& configuration) : configuration(configuration) {
  ApplicationManager& am = ApplicationManager::get_instance();
  logger = am.get_logger();
}

std::unique_ptr<FsPrediction> FsPrediction::get_instance(const FsConfiguration &configuration)
{
  std::unique_ptr<FsPrediction> fs_prediction;

  switch (configuration.prediction_type)
  {
  case StorageType::CSV:
    fs_prediction = std::unique_ptr<FsPrediction>(new CsvPredictionStorage(configuration));
    break;
  default:
    fs_prediction = std::unique_ptr<FsPrediction>(new CsvPredictionStorage(configuration));
  }

  return fs_prediction;
}
