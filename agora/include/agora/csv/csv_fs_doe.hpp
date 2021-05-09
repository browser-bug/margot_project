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

#ifndef CSV_FS_DOE_HPP
#define CSV_FS_DOE_HPP

#include <filesystem>
#include <string>

#include "agora/fs_doe.hpp"
#include "agora/logger.hpp"
#include "csv.hpp"

namespace agora {

/**
 * @brief Implementation of a FsDoe that manages DOE data via CSV files.
 */
class CsvDoeStorage : public FsDoe {
public:
    /**
     * @brief Construct a new instance.
     *
     * @param [in] configuration The FsConfiguration to use.
     *
     * @details
     * This constructor creates a new filesystem directory that will contains the DOE data inside the storage root directory specified
     * in the configuration.
     */
    CsvDoeStorage(const FsConfiguration &configuration);

    ~CsvDoeStorage() = default;

    /**
     * @brief Store the DOE data.
     *
     * @details
     * Create a single CSV file with the following header:
     *  - | config_id | counter | knob_1 | knob_2 | ... | knob_n | -> doe_configs.csv
     *
     * @see FsDoe::store_doe()
     */
    void store_doe(const application_id &app_id, const margot::heel::block_model &description, const doe_model &doe) override;
    /**
     * @brief Load the DOE data.
     *
     * @see FsDoe::load_doe()
     */
    doe_model load_doe(const application_id &app_id, const margot::heel::block_model &description) override;
    /**
     * @brief Update the specified DOE configuration by decreasing the number of observations that needs to be performed.
     *
     * @see FsDoe::update_doe()
     */
    void update_doe(const application_id &app_id, const margot::heel::block_model &description, const std::string &config_id) override;
    /**
     * @brief Remove all the DOE configurations still available.
     *
     * @see FsDoe::empty_doe_entries()
     */
    void empty_doe_entries(const application_id &app_id, const margot::heel::block_model &description) override;

    /**
     * @brief Get the filesystem path to the DOE data CSV file.
     *
     * @see FsDoe::get_doe_name()
     */
    std::string get_doe_name(const application_id &app_id) const override {
        std::filesystem::path p = doe_dir / app_id.path() / "doe_configs.csv";
        return p.string();
    }
    /**
     * @brief Get the filesystem path to the total configurations data CSV file.
     *
     * @see FsDoe::get_total_configurations_name()
     */
    std::string get_total_configurations_name(const application_id &app_id) const override {
        std::filesystem::path p = doe_dir / app_id.path() / "total_configs.csv";
        return p.string();
    }

    /**
     * @brief Delete the DOE data and the total configurations CSV files inside the storage directory.
     *
     * @see FsDoe::erase()
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
     * @brief The directory path containing all DOE data and the total configurations.
     */
    std::filesystem::path doe_dir;

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

#endif  // CSV_FS_DOE_HPP
