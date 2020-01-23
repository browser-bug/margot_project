/* mARGOt HEEL library
 * Copyright (C) 2018 Davide Gadioli
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