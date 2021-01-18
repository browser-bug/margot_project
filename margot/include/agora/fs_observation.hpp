#ifndef MARGOT_AGORA_FS_OBSERVATION_HPP
#define MARGOT_AGORA_FS_OBSERVATION_HPP

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include <heel/model_block.hpp>

#include "agora/fs_configuration.hpp"
#include "agora/logger.hpp"

namespace agora {

class FsObservation {

public:
  static std::unique_ptr<FsObservation> get_instance(const FsConfiguration& configuration);

  virtual void create_observation_table(const application_id &app_id, const margot::heel::block_model &description) = 0;
  virtual void insert_observation_entry(const application_id &app_id, const std::string &client_id,
                                        const long duration_sec, const long duration_ns,
                                        const margot::heel::operating_point_model &operating_point) = 0;

  virtual std::string get_observation_name(const application_id &app_id) const = 0;

  virtual void erase(const application_id &app_id) = 0;

  virtual std::string get_type() const = 0;

  virtual ~FsObservation() {}

protected:
  FsObservation(const FsConfiguration& configuration);
  FsConfiguration configuration;

  std::shared_ptr<Logger> logger;
};

} // namespace agora

#endif // FS_OBSERVATION_HPP
