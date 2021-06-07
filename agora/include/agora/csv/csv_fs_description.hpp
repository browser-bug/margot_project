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

#ifndef CSV_FS_DESCRIPTION_HPP
#define CSV_FS_DESCRIPTION_HPP

#include <filesystem>
#include <string>

#include "agora/fs_description.hpp"
#include "agora/logger.hpp"
#include "csv.hpp"

namespace agora {

/**
 * @brief Implementation of a FsDescription that manages application description data via CSV files.
 */
class CsvDescriptionStorage : public FsDescription {
public:
    /**
     * @brief Construct a new instance.
     *
     * @param [in] configuration The FsConfiguration to use.
     *
     * @details
     * This constructor creates a new filesystem directory that will contains the application information data inside the storage root
     * directory specified in the configuration.
     */
    CsvDescriptionStorage(const FsConfiguration &configuration);

    ~CsvDescriptionStorage() = default;

    /**
     * @brief Store the description data.
     *
     * @details
     * Create multiple CSV files with the following headers each:
     *  - | name | type | values | -> knobs.csv
     *  - | name | type | prediction_plugin | -> metrics.csv
     *  - | name | type | -> features.csv
     *  - | parameter_name | value | -> agora_properties.csv
     *  - | parameter_name | value | -> doe_parameters.csv
     *  - | parameter_name | value | -> clustering_parameters.csv
     *  - | parameter_name | value | -> <metric_name>_model_parameters.csv
     *
     * @see FsDescription::store_description()
     */
    void store_description(const application_id &app_id, const margot::heel::block_model &description) override;
    /**
     * @brief Load the description data.
     *
     * @see FsDescription::load_description()
     */
    margot::heel::block_model load_description(const application_id &app_id) override;

    /**
     * @brief Get the filesystem path to the software-knobs CSV file.
     *
     * @see FsDescription::get_knobs_name()
     */
    std::string get_knobs_name(const application_id &app_id) const override {
        std::filesystem::path p = description_dir / app_id.path() / "knobs.csv";
        return p.string();
    }
    /**
     * @brief Get the filesystem path to the input features CSV file.
     *
     * @see FsDescription::get_features_name()
     */
    std::string get_features_name(const application_id &app_id) const override {
        std::filesystem::path p = description_dir / app_id.path() / "features.csv";
        return p.string();
    }
    /**
     * @brief Get the filesystem path to the EFPs CSV file.
     *
     * @see FsDescription::get_metrics_name()
     */
    std::string get_metrics_name(const application_id &app_id) const override {
        std::filesystem::path p = description_dir / app_id.path() / "metrics.csv";
        return p.string();
    }
    /**
     * @brief Get the filesystem path to the agora properties CSV file.
     *
     * @see FsDescription::get_properties_name()
     */
    std::string get_properties_name(const application_id &app_id) const override {
        std::filesystem::path p = description_dir / app_id.path() / "agora_properties.csv";
        return p.string();
    }
    /**
     * @brief Get the filesystem path to the DOE parameters CSV file.
     *
     * @see FsDescription::get_doe_parameters_name()
     */
    std::string get_doe_parameters_name(const application_id &app_id) const override {
        std::filesystem::path p = description_dir / app_id.path() / "doe_parameters.csv";
        return p.string();
    }
    /**
     * @brief Get the filesystem path to the modelling parameters CSV file.
     *
     * @see FsDescription::get_model_parameters_name()
     */
    std::string get_model_parameters_name(const application_id &app_id, const std::string &metric_name) const override {
        std::filesystem::path p = description_dir / app_id.path() / std::string(metric_name + "_model_parameters.csv");
        return p.string();
    }
    /**
     * @brief Get the filesystem path to the clustering parameters CSV file.
     *
     * @see FsDescription::get_clustering_parameters_name()
     */
    std::string get_clustering_parameters_name(const application_id &app_id) const override {
        std::filesystem::path p = description_dir / app_id.path() / "clustering_parameters.csv";
        return p.string();
    }

    /**
     * @brief Delete the description data CSV files inside the storage directory.
     *
     * @see FsDescription::erase()
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
     * @brief The directory path containing all application description data.
     */
    std::filesystem::path description_dir;

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

#endif  // CSV_FS_DESCRIPTION_HPP
