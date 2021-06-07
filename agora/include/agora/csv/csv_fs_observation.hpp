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

/**
 * @brief Implementation of a FsObservation that manages observations data via CSV files.
 */
class CsvObservationStorage : public FsObservation {
public:
    /**
     * @brief Construct a new instance.
     *
     * @param [in] configuration The FsConfiguration to use.
     *
     * @details
     * This constructor creates a new filesystem directory that will contains the observations data inside the storage root directory
     * specified in the configuration.
     */
    CsvObservationStorage(const FsConfiguration &configuration);

    ~CsvObservationStorage() = default;

    /**
     * @brief Create a new CSV file for the observations of the specified application.
     *
     * @details
     * Create a single CSV file with the following header:
     *  - | sec | nanosec | client_id | knob_1 | ... | knob_n | feature_1 | ... | feature_n | metric_1 | ... | metric_n | ->
     * observations.csv
     *
     * @see FsObservation::create_observation_table()
     */
    void create_observation_table(const application_id &app_id, const margot::heel::block_model &description) override;
    /**
     * @brief Append a new observation to the CSV file.
     *
     * @note
     * Since this is a high frequency operation, the number of accesses to the CSV file can be very high. Appending a row to a CSV file is
     * not a thread-safe operation, hence a mutex is needed in order to manage the critical section.
     *
     * @see FsObservation::insert_observation_entry()
     */
    void insert_observation_entry(const application_id &app_id, const std::string &client_id, long duration_sec, long duration_ns,
                                  const margot::heel::operating_point_model &operating_point) override;

    /**
     * @brief Get the filesystem path to the observations data CSV file.
     *
     * @see FsObservation::get_observation_name()
     */
    std::string get_observation_name(const application_id &app_id) const override {
        std::filesystem::path p = observation_dir / app_id.path() / "observations.csv";
        return p.string();
    }

    /**
     * @brief Delete the observations data CSV files inside the storage directory.
     *
     * @see FsObservation::erase()
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
     * @brief The directory path containing all observations data.
     */
    std::filesystem::path observation_dir;
    /**
     * @brief The mutex used to enforce a synchronization on the observations insertion.
     */
    std::mutex mtx;

    /**
     * @brief The format used inside the CSV files.
     */
    csv::CSVFormat format;
    /**
     * @brief The column separator used inside the CSV files.
     */
    const char csv_separator;

    /**
     * @brief Get the observations data CSV file header.
     *
     * @param [in] app_id The AID corresponding to the application.
     * @param [in] description The application description containing software-knobs, input features and EFPs to store.
     */
    std::string get_header(const application_id &app_id, const margot::heel::block_model &description);
};

}  // namespace agora

#endif  // CSV_FS_OBSERVATION_HPP
