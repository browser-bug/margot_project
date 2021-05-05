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

class CsvClusterStorage : public FsCluster {
public:
    CsvClusterStorage(const FsConfiguration &configuration);

    ~CsvClusterStorage() = default;

    void store_cluster(const application_id &app_id, const margot::heel::block_model &description, const cluster_model &cluster) override;
    cluster_model load_cluster(const application_id &app_id, const margot::heel::block_model &description) override;

    void erase(const application_id &app_id) override;

    // the followings get the relative path for each specific table
    std::string get_cluster_name(const application_id &app_id) const override {
        std::filesystem::path p = cluster_dir / app_id.path() / "centroids.csv";
        return p.string();
    }

    std::string get_type() const override { return "csv"; }

private:
    // this path will contain all the stored information
    std::filesystem::path cluster_dir;

    // configuration variables, for handling csv parsing
    csv::CSVFormat format;
    const char csv_separator;
};

}  // namespace agora

#endif  // CSV_FS_CLUSTER_HPP
