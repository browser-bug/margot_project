#ifndef HEEL_MODEL_APPLICATION_HDR
#define HEEL_MODEL_APPLICATION_HDR

#include <string>
#include <vector>

#include <heel/model/block.hpp>

namespace margot {
namespace heel {

struct application_model {
  std::string name;
  std::string version;
  std::vector<block_model> blocks;
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_APPLICATION_HDR