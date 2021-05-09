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
#include "agora/csv/csv_fs_observation.hpp"
#include "agora/csv/csv.hpp"

namespace agora {

namespace fs = std::filesystem;

CsvObservationStorage::CsvObservationStorage(const FsConfiguration &configuration)
        : FsObservation(configuration), csv_separator(configuration.csv_separator) {
    observation_dir = configuration.csv_storage_root_path / "observations";
    fs::create_directories(observation_dir);

    // add formatting settings to the csv parser
    format.delimiter(csv_separator);
}

void CsvObservationStorage::create_observation_table(const application_id &app_id, const margot::heel::block_model &description) {
    fs::create_directories(observation_dir / app_id.path());

    std::ostringstream ss;
    ss << get_header(app_id, description);

    // open the file for the observation table
    std::ofstream out;
    std::lock_guard<std::mutex> lock(mtx);
    out.open(get_observation_name(app_id), std::ios::out | std::ios::trunc);
    out << ss.str();
}

void CsvObservationStorage::insert_observation_entry(const application_id &app_id, const std::string &client_id, const long duration_sec,
                                                     const long duration_ns, const margot::heel::operating_point_model &operating_point) {
    std::ostringstream ss;
    ss << duration_sec << ',' << duration_ns << ',' << client_id;

    for (const auto &knob : operating_point.knobs) {
        ss << ',' << knob.mean;
    }

    for (const auto &feature : operating_point.features) {
        ss << ',' << feature.mean;
    }

    for (const auto &metric : operating_point.metrics) {
        ss << ',' << metric.mean;
    }

    ss << "\n";

    // open the file for the observation table and here we're just appending to avoid the annoying csv entire rewrite
    std::ofstream out;
    std::lock_guard<std::mutex> lock(mtx);
    out.open(get_observation_name(app_id), std::ios::out | std::ios::app);
    out << ss.str();
    ;
}

std::string CsvObservationStorage::get_header(const application_id &app_id, const margot::heel::block_model &description) {
    std::ostringstream ss;

    // write the common part of the header
    ss << "sec,nanosec,client_id";
    // write the software knobs
    for (const auto &knob : description.knobs) {
        ss << ',' << knob.name;
    }

    // write the input features
    for (const auto &feature : description.features.fields) {
        ss << ',' << feature.name;
    }

    // write the metrics
    for (const auto &metric : description.metrics) {
        ss << ',' << metric.name;
    }
    ss << "\n";

    return ss.str();
}

void CsvObservationStorage::erase(const application_id &app_id) {
    std::error_code ec;
    auto path = observation_dir / app_id.path();
    fs::remove_all(path, ec);
    if (ec.value() != 0) logger->warning("Csv manager: unable to remove \"", path.string(), "\", err: ", ec.message());
}

}  // namespace agora
