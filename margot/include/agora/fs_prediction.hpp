#ifndef MARGOT_AGORA_FS_PREDICTION_HPP
#define MARGOT_AGORA_FS_PREDICTION_HPP

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include <heel/model_block.hpp>

#include "agora/fs_configuration.hpp"
#include "agora/model_prediction.hpp"

namespace agora {

class FsPrediction {

public:
  static std::unique_ptr<FsPrediction> get_instance(const FsConfiguration& configuration);

  virtual void store_prediction(const application_id &app_id, const margot::heel::block_model &description, const prediction_model &prediction) = 0;
  virtual prediction_model load_prediction(const application_id &app_id, const margot::heel::block_model &description) = 0;

  virtual std::string get_prediction_name(const application_id &app_id) const = 0;

  virtual void erase(const application_id &app_id, const margot::heel::block_model &description) = 0;

  virtual std::string get_type() const = 0;

  virtual ~FsPrediction() {}

protected:
  FsPrediction(const FsConfiguration& configuration);
  FsConfiguration configuration;

  std::shared_ptr<Logger> logger;
};

} // namespace agora

#endif // FS_PREDICTION_HPP
