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

class CsvPredictionStorage : public FsPrediction{

public:
  CsvPredictionStorage(const FsConfiguration& configuration);

  ~CsvPredictionStorage() = default;

  void store_prediction(const application_id &app_id,const margot::heel::block_model &description, const prediction_model &model) override;
  prediction_model load_prediction(const application_id &app_id,const margot::heel::block_model &description) override;

  void erase(const application_id &app_id) override;

  // the followings get the relative path for each specific table
  std::string get_prediction_name(const application_id &app_id) const override
  {
    std::filesystem::path p = prediction_dir / app_id.path() / "predictions.csv";
    return p.string();
  }

  std::string get_type() const override {
    return "csv";
  }

private:
  // this path will contain all the stored information
  std::filesystem::path prediction_dir;

  // configuration variables, for handling csv parsing
  const char csv_separator;
  csv::CSVFormat format;
};

} // namespace agora

#endif // CSV_FS_PREDICTION_HPP
