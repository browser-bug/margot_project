#ifndef HEEL_JSON_PARSER_HDR
#define HEEL_JSON_PARSER_HDR

#include <heel/configuration_file.hpp>
#include <heel/model_application.hpp>
#include <heel/parser_application.hpp>
#include <heel/parser_operating_point.hpp>

namespace margot {
namespace heel {

inline application_model parse_json(const configuration_file& conf_file) {
  return parse_application(conf_file.ptree());
}

inline void parse_json(const configuration_file& conf_file, application_model& application) {
  parse_operating_points(conf_file.ptree(), application);
}

}  // namespace heel
}  // namespace margot

#endif  // HEEL_JSON_PARSER_HDR