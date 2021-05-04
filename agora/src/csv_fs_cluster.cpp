/* Agora library
 * Copyright (C) 2021 Bernardo Menicagli
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

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

  try
  {
    // open the table of the cluster
    csv::CSVReader cluster_parser(get_cluster_name(app_id), format);

    // go through each line of cluster
    for (auto &row : cluster_parser)
    {
      const std::string id = row["centroid_id"].get();

      centroid_model centroid;
      for (const auto &feature : description.features.fields)
      {
        centroid.push_back(row[feature.name].get());
      }
      output_cluster.add_centroid(id, centroid);
    }
  } catch (const std::exception &e)
  {
    logger->warning("Csv cluster: error [", e.what(), "]. Returning an empty cluster.");
  }

  return output_cluster;
}

void CsvClusterStorage::erase(const application_id &app_id)
{
  std::error_code ec;
  auto path = cluster_dir / app_id.path();
  fs::remove_all(path, ec);
  if (ec.value() != 0)
    logger->warning("Csv manager: unable to remove \"", path.string(), "\", err: ", ec.message());
}

} // namespace agora
