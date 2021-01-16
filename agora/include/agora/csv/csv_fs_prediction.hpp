#ifndef CSV_FS_PREDICTION_HPP
#define CSV_FS_PREDICTION_HPP

#include <filesystem>
#include <string>

#include "agora/fs_prediction.hpp"
#include "agora/logger.hpp"
#include "csv.hpp"

namespace agora {

class CsvPredictionStorage : public FsPrediction{

public:
  CsvPredictionStorage(const FsConfiguration& configuration);

  ~CsvPredictionStorage() = default;

  void store_prediction(const application_id &app_id,const margot::heel::block_model &description, const prediction_model &model) override;
  prediction_model load_prediction(const application_id &app_id,const margot::heel::block_model &description) override;

  void erase(const application_id &app_id, const margot::heel::block_model &description) override;

  // the followings get the relative path for each specific table
  std::string get_prediction_name(const application_id &app_id) const override
  {
    std::filesystem::path p = prediction_dir / app_id.path() / "predictions.csv";
    return p.string();
  }
  std::string get_model_name(const application_id &app_id, const std::string &metric_name) const override
  {
    std::filesystem::path p = model_dir / app_id.path() / std::string(metric_name + "_model.data");
    return p.string();
  }

  std::string get_type() const override {
    return "csv";
  }

private:
  // this path will contain all the stored information
  std::filesystem::path prediction_dir;
  std::filesystem::path model_dir;

  // configuration variables, for handling csv parsing
  const char csv_separator;
  csv::CSVFormat format;

  void safe_rm(const std::filesystem::path &file_path);
};

} // namespace agora

#endif // CSV_FS_PREDICTION_HPP
