#ifndef CSV_FS_DOE_HPP
#define CSV_FS_DOE_HPP

#include <filesystem>
#include <string>

#include "agora/fs_doe.hpp"
#include "agora/logger.hpp"
#include "agora/csv/csv.hpp"

namespace agora {

class CsvDoeStorage : public FsDoe {

public:
  CsvDoeStorage(const FsConfiguration& configuration);

  ~CsvDoeStorage() = default;

  void store_doe(const application_id &app_id, const margot::heel::block_model &description, const doe_model &doe) override;
  doe_model load_doe(const application_id &app_id, const margot::heel::block_model &description) override;
  void update_doe(const application_id &app_id, const margot::heel::block_model &description, const std::string &config_id) override;
  void empty_doe_entries(const application_id &app_id, const margot::heel::block_model &description) override;

  // the followings get the relative path for each specific table
  std::string get_doe_name(const application_id& app_id) const override
  {
    std::filesystem::path p = doe_dir / app_id.path() / "doe_configs.csv";
    return p.string();
  }
  std::string get_total_configurations_name(const application_id& app_id) const override
  {
    std::filesystem::path p = doe_dir / app_id.path() / "total_configs.csv";
    return p.string();
  }

  void erase(const application_id& app_id) override;

  std::string get_type() const override { return "csv"; }

private:
  // this path will contain all the stored information
  std::filesystem::path doe_dir;

  // configuration variables, for handling csv parsing
  const char csv_separator;
  csv::CSVFormat format;

  void safe_rm(const std::filesystem::path &file_path);
};

} // namespace agora

#endif // CSV_FS_DOE_HPP
