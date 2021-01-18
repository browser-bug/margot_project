#ifndef CSV_FS_OBSERVATION_HPP
#define CSV_FS_OBSERVATION_HPP

#include <filesystem>
#include <string>

#include "agora/fs_observation.hpp"
#include "agora/logger.hpp"
#include "csv.hpp"

namespace agora {

class CsvObservationStorage : public FsObservation {

public:
  CsvObservationStorage(const FsConfiguration& configuration);

  ~CsvObservationStorage() = default;

  void create_observation_table(const application_id &app_id, const margot::heel::block_model &description) override;
  void insert_observation_entry(const application_id &application_id, const std::string &client_id, const long duration_sec,
                                const long duration_ns, const margot::heel::operating_point_model &operating_point) override;

  void erase(const application_id &app_id) override;

  // the followings get the relative path for each specific table
  std::string get_observation_name(const application_id &app_id) const override
  {
    std::filesystem::path p = observation_dir / app_id.path() / "observations.csv";
    return p.string();
  }

  std::string get_type() const override { return "csv"; }

private:
  // this path will contain all the stored information
  std::filesystem::path observation_dir;

  // configuration variables, for handling csv parsing
  const char csv_separator;
  csv::CSVFormat format;

  void safe_rm(const std::filesystem::path &file_path);
};

} // namespace agora

#endif // CSV_FS_OBSERVATION_HPP
