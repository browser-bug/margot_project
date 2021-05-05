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

#include "agora/fs_model.hpp"
#include "agora/application_manager.hpp"

namespace agora {

namespace fs = std::filesystem;

FsModel::FsModel(const FsConfiguration &configuration) : configuration(configuration), model_dir(configuration.model_storage_root_path) {
    ApplicationManager &am = ApplicationManager::get_instance();
    logger = am.get_logger();
}

void FsModel::erase(const application_id &app_id) {
    std::error_code ec;
    auto path = model_dir / app_id.path();
    fs::remove_all(path, ec);
    if (ec.value() != 0) logger->warning("Model manager: unable to remove \"", path.string(), "\", err: ", ec.message());
}

}  // namespace agora
