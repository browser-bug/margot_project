#include "agora/fs_cluster.hpp"
#include "agora/csv/csv_fs_cluster.hpp"
#include "agora/application_manager.hpp"

using namespace agora;

FsCluster::FsCluster(const FsConfiguration &configuration) : configuration(configuration)
{
  ApplicationManager &am = ApplicationManager::get_instance();
  logger = am.get_logger();
}

std::unique_ptr<FsCluster> FsCluster::get_instance(const FsConfiguration &configuration)
{
  std::unique_ptr<FsCluster> fs_cluster;

  switch (configuration.cluster_type)
  {
  case StorageType::CSV:
    fs_cluster = std::unique_ptr<FsCluster>(new CsvClusterStorage(configuration));
    break;
  default:
    fs_cluster = std::unique_ptr<FsCluster>(new CsvClusterStorage(configuration));
  }

  return fs_cluster;
}
