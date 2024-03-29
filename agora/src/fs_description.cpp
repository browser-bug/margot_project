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

#include "agora/fs_description.hpp"
#include "agora/csv/csv_fs_description.hpp"
#include "agora/application_manager.hpp"

using namespace agora;

FsDescription::FsDescription(const FsConfiguration &configuration) : configuration(configuration) {
    ApplicationManager &am = ApplicationManager::get_instance();
    logger = am.get_logger();
}

std::unique_ptr<FsDescription> FsDescription::get_instance(const FsConfiguration &configuration) {
    std::unique_ptr<FsDescription> fs_description;

    switch (configuration.description_type) {
        case StorageType::CSV:
            fs_description = std::unique_ptr<FsDescription>(new CsvDescriptionStorage(configuration));
            break;
        default:
            fs_description = std::unique_ptr<FsDescription>(new CsvDescriptionStorage(configuration));
    }

    return fs_description;
}
