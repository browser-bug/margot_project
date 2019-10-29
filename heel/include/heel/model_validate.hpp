#ifndef HEEL_MODEL_VALIDATION_HDR
#define HEEL_MODEL_VALIDATION_HDR

#include <heel/model_application.hpp>

namespace margot {
namespace heel {

// this function post-process the model and it tries to guess any missing information. If it founds a
// non-recoverable error, it will terminate the execution of the application.
void validate(application_model& model);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_MODEL_VALIDATION_HDR