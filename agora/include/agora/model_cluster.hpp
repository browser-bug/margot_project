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

#ifndef MODEL_CLUSTER_HPP
#define MODEL_CLUSTER_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "agora/logger.hpp"

namespace agora {

/**
 * @brief A vector representing a generic input features cluster centroid.
 *
 * @details
 * In table format this can be represented as | feat_1 | ... | feat_n |
 */
using centroid_model = std::vector<std::string>;

/**
 * @brief A data structure representing the output of the clustering plugin.
 *
 * @details
 * The output is seen as a list of input features cluster centroids.
 */
struct cluster_model {
    /**
     * @brief Add a new centroid.
     *
     * @param [in] centroid_id The unique identification number of the centroid.
     * @param [in] centroid The centroid values (i.e. a series of input features values).
     *
     * @returns True if the centroid_id was seen for the first time, False if the it was already present and hence substituted with a new
     *  value.
     */
    bool add_centroid(const std::string &centroid_id, const centroid_model &centroid) {
        return !centroids.insert_or_assign(centroid_id, centroid).second;
    }

    /**
     * @brief Remove the specified centroid.
     *
     * @param [in] centroid_id The unique identification number of the centroid.
     */
    void remove_centroid(const std::string &centroid_id) { centroids.erase(centroid_id); }

    /**
     * @brief Remove all the centroids.
     */
    void clear() { centroids.clear(); }

    /**
     * @brief A list of centroids.
     *
     * @details
     * Each element is seen as a (key, value) pair:
     *  - Key: the centroid ID.
     *  - Value: the centroid values.
     */
    std::unordered_map<std::string, centroid_model> centroids;
};

}  // namespace agora

#endif // MODEL_CLUSTER_HPP
