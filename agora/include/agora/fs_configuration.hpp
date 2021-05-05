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

enum class StorageType { CSV };

struct FsConfiguration {
    FsConfiguration()
            : description_type(StorageType::CSV),
              doe_type(StorageType::CSV),
              cluster_type(StorageType::CSV),
              prediction_type(StorageType::CSV),
              observation_type(StorageType::CSV) {}

    void set_csv_handler_properties(const std::filesystem::path &root_path, const char &separator) {
        csv_storage_root_path = root_path;
        csv_separator = separator;
    }

    void set_model_handler_properties(const std::filesystem::path &root_path) { model_storage_root_path = root_path; }

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

}  // namespace agora

#endif  // FS_CONFIGURATION_HPP
