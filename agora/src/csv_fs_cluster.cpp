#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "agora/agora_properties.hpp"
#include "agora/csv/csv_fs_cluster.hpp"
#include "agora/csv/csv.hpp"

namespace agora {

namespace fs = std::filesystem;

CsvClusterStorage::CsvClusterStorage(const FsConfiguration& configuation)
    : FsCluster(configuation), csv_separator(configuation.csv_separator)
{
  cluster_dir = configuation.csv_storage_root_path / "clusters";
  fs::create_directories(cluster_dir);

  // add formatting settings to the csv parser
  format.delimiter(csv_separator);
}

void CsvClusterStorage::store_cluster(const application_id &app_id, const margot::heel::block_model &description, const cluster_model &cluster)
{
  fs::create_directories(cluster_dir / app_id.path());

  // open the file for the clusters table
  std::ofstream out;
  out.open(get_cluster_name(app_id), std::ios::out | std::ios::trunc);

  // write the header
  out << "centroid_id";
  for (const auto &feature : description.features.fields)
  {
    out << ',' << feature.name;
  }
  out << "\n";

  // write all the centroids
  for (const auto &centroid : cluster.centroids)
  {
    out << centroid.first;
    for (const auto &value : centroid.second)
      out << ',' << value;
    out << "\n";
  }
}

cluster_model CsvClusterStorage::load_cluster(const application_id &app_id,const margot::heel::block_model &description)
{
  // declare the output cluster
  cluster_model output_cluster;

  // open the table of the cluster
  csv::CSVReader cluster_parser(get_cluster_name(app_id), format);

  // go through each line of cluster
  for (auto &row : cluster_parser)
  {
    const std::string id = row["centroid_id"].get();

    centroid_model centroid;
    for(const auto& feature : description.features.fields)
    {
      centroid.push_back(row[feature.name].get());
    }
    output_cluster.add_centroid(id, centroid);
  }

  return output_cluster;
}

// declare a function to safely remove a file
void CsvClusterStorage::safe_rm(const fs::path &file_path)
{
  std::error_code ec;
  fs::remove(file_path, ec);
  if (ec.value() != 0)
    logger->warning("Csv manager: unable to remove the file \"", file_path.string(), "\", err: ", ec.message());
};

void CsvClusterStorage::erase(const application_id &app_id)
{
  safe_rm(get_cluster_name(app_id));
}

} // namespace agora
