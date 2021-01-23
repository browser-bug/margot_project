#ifndef FS_CONFIGURATION_HPP
#define FS_CONFIGURATION_HPP

#include <filesystem>

namespace agora {

enum class StorageType { CSV };

struct FsConfiguration
{
  FsConfiguration()
      : description_type(StorageType::CSV), doe_type(StorageType::CSV), cluster_type(StorageType::CSV), prediction_type(StorageType::CSV),
        observation_type(StorageType::CSV)
  {}

  void set_csv_handler_properties(const std::filesystem::path &root_path, const char &separator)
  {
    csv_storage_root_path = root_path;
    csv_separator = separator;
  }

  void set_model_handler_properties(const std::filesystem::path &root_path)
  {
    model_storage_root_path = root_path;
  }

  StorageType description_type;
  StorageType doe_type;
  StorageType cluster_type;
  StorageType prediction_type;
  StorageType observation_type;

  // csv handler
  std::filesystem::path csv_storage_root_path;
  char csv_separator;

  // models handler
  std::filesystem::path model_storage_root_path;
};

} // namespace agora

#endif // FS_CONFIGURATION_HPP
