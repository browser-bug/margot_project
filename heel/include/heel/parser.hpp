#ifndef HEEL_PARSER_HDR
#define HEEL_PARSER_HDR

#include <vector>

#include <heel/configuration_file.hpp>
#include <heel/model_application.hpp>
#include <heel/model_block.hpp>
#include <heel/parser_application.hpp>
#include <heel/parser_operating_point.hpp>

namespace margot {
namespace heel {

inline application_model parse(const configuration_file& conf_file) {
  return parse_application(conf_file.ptree());
}

inline std::vector<operating_point_model> parse(const configuration_file& conf_file,
                                                const block_model& block) {
  return parse_operating_points(conf_file.ptree(), block);
}

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_HDR