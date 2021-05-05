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

#ifndef MARGOT_AGORA_MODEL_CLUSTER_HPP
#define MARGOT_AGORA_MODEL_CLUSTER_HPP

#include <string>
#include <unordered_map>
#include <vector>

#include "agora/logger.hpp"

namespace agora {

//
// | FEAT1 | FEAT2 | ... | FEATN
//
// TODO: this can become something more detailed inside heel
using centroid_model = std::vector<std::string>;

struct cluster_model {
    // this method adds a centroid
    bool add_centroid(const std::string &centroid_id, const centroid_model &centroid) {
        bool assignment_took_place =
                !centroids.insert_or_assign(centroid_id, centroid).second || !centroids.insert_or_assign(centroid_id, centroid).second;
        return assignment_took_place;
    }

    // this method removes a centroid
    void remove_centroid(const std::string &centroid_id) { centroids.erase(centroid_id); }

    void clear() { centroids.clear(); }

    std::unordered_map<std::string, centroid_model> centroids;
};

}  // namespace agora

#endif  // MODEL_CLUSTER_HPP
