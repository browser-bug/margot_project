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

#ifndef CSV_FS_CLUSTER_HPP
#define CSV_FS_CLUSTER_HPP

#include <filesystem>
#include <string>

#include "agora/fs_cluster.hpp"
#include "agora/logger.hpp"
#include "csv.hpp"

namespace agora {

/**
 * @brief Implementation of a FsCluster that manages input features cluster data via CSV files.
 */
class CsvClusterStorage : public FsCluster {
public:
    /**
     * @brief Construct a new instance.
     *
     * @param [in] configuration The FsConfiguration to use.
     *
     * @details
     * This constructor creates a new filesystem directory that will contains the cluster data inside the storage root directory specified
     * in the configuration.
     */
    CsvClusterStorage(const FsConfiguration &configuration);

    ~CsvClusterStorage() = default;

    /**
     * @brief Store the cluster data.
     *
     * @details
     * Create a single CSV file with the following header:
     *  - | centroid_id | feature_1 | feature_2 | ... | feature_n | -> centroids.csv
     *
     * @see FsCluster::store_cluster()
     */
    void store_cluster(const application_id &app_id, const margot::heel::block_model &description, const cluster_model &cluster) override;
    /**
     * @brief Load the cluster data.
     *
     * @see FsCluster::load_cluster()
     */
    cluster_model load_cluster(const application_id &app_id, const margot::heel::block_model &description) override;

    /**
     * @brief Get the filesystem path to the cluster data CSV file.
     *
     * @see FsCluster::get_cluster_name()
     */
    std::string get_cluster_name(const application_id &app_id) const override {
        std::filesystem::path p = cluster_dir / app_id.path() / "centroids.csv";
        return p.string();
    }

    /**
     * @brief Delete the cluster data CSV files inside the storage directory.
     *
     * @see FsCluster::erase()
     */
    void erase(const application_id &app_id) override;

    /**
     * @brief Get the storage type.
     *
     * @returns A string containing "csv".
     */
    std::string get_type() const override { return "csv"; }

private:
    /**
     * @brief The directory path containing all input features cluster data.
     */
    std::filesystem::path cluster_dir;

    /**
     * @brief The format used inside the CSV files.
     */
    csv::CSVFormat format;
    /**
     * @brief The column separator used inside the CSV files.
     */
    const char csv_separator;
};

}  // namespace agora

#endif  // CSV_FS_CLUSTER_HPP
