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

#ifndef MARGOT_AGORA_FS_DOE_HPP
#define MARGOT_AGORA_FS_DOE_HPP

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include <heel/model_block.hpp>

#include "agora/fs_configuration.hpp"
#include "agora/model_doe.hpp"

namespace agora {

class FsDoe {
public:
    static std::unique_ptr<FsDoe> get_instance(const FsConfiguration &configuration);

    virtual void store_doe(const application_id &app_id, const margot::heel::block_model &description, const doe_model &doe) = 0;
    virtual doe_model load_doe(const application_id &app_id, const margot::heel::block_model &description) = 0;
    virtual void update_doe(const application_id &app_id, const margot::heel::block_model &description, const std::string &config_id) = 0;
    virtual void empty_doe_entries(const application_id &app_id, const margot::heel::block_model &description) = 0;

    virtual std::string get_doe_name(const application_id &app_id) const = 0;
    virtual std::string get_total_configurations_name(const application_id &app_id) const = 0;

    virtual void erase(const application_id &app_id) = 0;

    virtual std::string get_type() const = 0;

    virtual ~FsDoe() {}

protected:
    FsDoe(const FsConfiguration &configuration);
    FsConfiguration configuration;

    std::shared_ptr<Logger> logger;
};

}  // namespace agora

#endif  // FS_DOE_HPP
