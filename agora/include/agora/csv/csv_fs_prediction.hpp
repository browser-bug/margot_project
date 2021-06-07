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

#ifndef CSV_FS_PREDICTION_HPP
#define CSV_FS_PREDICTION_HPP

#include <filesystem>
#include <string>

#include "agora/fs_prediction.hpp"
#include "agora/logger.hpp"
#include "csv.hpp"

namespace agora {

/**
 * @brief Implementation of a FsPrediction that manages input predictions data via CSV files.
 */
class CsvPredictionStorage : public FsPrediction {
public:
    /**
     * @brief Construct a new instance.
     *
     * @param [in] configuration The FsConfiguration to use.
     *
     * @details
     * This constructor creates a new filesystem directory that will contains the predictions data inside the storage root directory
     * specified in the configuration.
     */
    CsvPredictionStorage(const FsConfiguration &configuration);

    ~CsvPredictionStorage() = default;

    /**
     * @brief Store the predictions data.
     *
     * @details
     * Create a single CSV file with the following header:
     *  - | pred_id | knob_1 | ... | knob_n | feature_1 | ... | feature_n | metric_1_avg | metric_1_std | ... | metric_n_avg | ... |
     * metric_n_std | -> predictions.csv
     *
     * @see FsPrediction::store_prediction()
     */
    void store_prediction(const application_id &app_id, const margot::heel::block_model &description,
                          const prediction_model &prediction) override;
    /**
     * @brief Load the predictions data.
     *
     * @see FsPrediction::load_prediction()
     */
    prediction_model load_prediction(const application_id &app_id, const margot::heel::block_model &description) override;

    /**
     * @brief Get the filesystem path to the predictions data CSV file.
     *
     * @see FsPrediction::get_prediction_name()
     */
    std::string get_prediction_name(const application_id &app_id) const override {
        std::filesystem::path p = prediction_dir / app_id.path() / "predictions.csv";
        return p.string();
    }

    /**
     * @brief Delete the cluster data CSV files inside the storage directory.
     *
     * @see FsPrediction::erase()
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
     * @brief The directory path containing all predictions data.
     */
    std::filesystem::path prediction_dir;

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

#endif  // CSV_FS_PREDICTION_HPP
