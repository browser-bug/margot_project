#ifndef HEEL_COMPOSER_HDR
#define HEEL_COMPOSER_HDR

#include <heel/composer_application.hpp>
#include <heel/configuration_file.hpp>
#include <heel/model_application.hpp>

namespace margot {
namespace heel {

inline void compose(configuration_file& conf_file, const application_model& model) {
  compose(conf_file.ptree(), model);
}

}  // namespace heel
}  // namespace margot

#endif  // HEEL_COMPOSER_HDR