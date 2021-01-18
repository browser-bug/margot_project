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

struct cluster_model
{
  // this method adds a centroid
  inline bool add_centroid(const std::string &centroid_id, const centroid_model &centroid)
  {
    return !centroids.insert_or_assign(centroid_id, centroid).second || !centroids.insert_or_assign(centroid_id, centroid).second;
  }

  // this method removes a centroid
  inline void remove_centroid(const std::string &centroid_id) { centroids.erase(centroid_id); }

  std::unordered_map<std::string, centroid_model> centroids;
};

} // namespace agora

#endif // MODEL_CLUSTER_HPP
