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

#ifndef CSV_FS_OBSERVATION_HPP
#define CSV_FS_OBSERVATION_HPP

#include <filesystem>
#include <string>

#include "agora/fs_observation.hpp"
#include "agora/logger.hpp"
#include "csv.hpp"

namespace agora {

class CsvObservationStorage : public FsObservation {
public:
    CsvObservationStorage(const FsConfiguration &configuration);

    ~CsvObservationStorage() = default;

    void create_observation_table(const application_id &app_id, const margot::heel::block_model &description) override;
    void insert_observation_entry(const application_id &app_id, const std::string &client_id, long duration_sec, long duration_ns,
                                  const margot::heel::operating_point_model &operating_point) override;

    void erase(const application_id &app_id) override;

    // the followings get the relative path for each specific table
    std::string get_observation_name(const application_id &app_id) const override {
        std::filesystem::path p = observation_dir / app_id.path() / "observations.csv";
        return p.string();
    }

    std::string get_type() const override { return "csv"; }

private:
    // this path will contain all the stored information
    std::filesystem::path observation_dir;
    std::mutex mtx;

    // configuration variables, for handling csv parsing
    const char csv_separator;
    csv::CSVFormat format;

    std::string get_header(const application_id &app_id, const margot::heel::block_model &description);
};

}  // namespace agora

#endif  // CSV_FS_OBSERVATION_HPP
