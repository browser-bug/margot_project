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

#ifndef FS_CONFIGURATION_HPP
#define FS_CONFIGURATION_HPP

#include <filesystem>

namespace agora {

/**
 * @brief Available storage implementations.
 *
 * @details
 * These values represents a list of available storage implementations which specifies a generic storage handler.
 */
enum class StorageType {
    CSV  ///< Comma-Separated Values files.
};

/**
 * @brief A generic configuration for a storage handler.
 *
 * @details
 * This data structure contains the specification for each section of data inside Agora. This enables modularity in choosing the type of
 * implementation to use for each of them independently.
 * Besides this, the configuration contains the filesystem root path in which the final models will be stored (i.e. the destination folder
 * in which the Modelling plugin will store the best computed model).
 */
struct FsConfiguration {
    // TODO: this defaults to CSV for each of the sections. For future developments, a parameters list needs to be supplied specifying each
    // of them.
    FsConfiguration()
            : description_type(StorageType::CSV),
              doe_type(StorageType::CSV),
              cluster_type(StorageType::CSV),
              prediction_type(StorageType::CSV),
              observation_type(StorageType::CSV) {}

    /**
     * @brief Set the properties of a generic CSV handler.
     *
     * @param [in] root_path The filesystem path where the CSV files will be stored.
     * @param [in] separator The separator character which identify different columns.
     */
    void set_csv_handler_properties(const std::filesystem::path &root_path, const char &separator) {
        csv_storage_root_path = root_path;
        csv_separator = separator;
    }

    /**
     * @brief Set the properties for the storage containing the final models.
     *
     * @param[in] root_path The filesystem path where the models will be stored.
     */
    void set_model_handler_properties(const std::filesystem::path &root_path) { model_storage_root_path = root_path; }

    /// The storage type for application description data.
    StorageType description_type;
    /// The storage type for DOE data.
    StorageType doe_type;
    /// The storage type for input features cluster data.
    StorageType cluster_type;
    /// The storage type for predictions data.
    StorageType prediction_type;
    /// The storage type for observations data.
    StorageType observation_type;

    /// The filesystem root path for CSV files.
    std::filesystem::path csv_storage_root_path;
    /// The CSV separator character to distinguish columns.
    char csv_separator;

    /// The filesystem root path for the final models.
    std::filesystem::path model_storage_root_path;
};

}  // namespace agora

#endif // FS_CONFIGURATION_HPP
