/* mARGOt HEEL library
 * Copyright (C) 2018 Politecnico di Milano
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

#ifndef HEEL_GENERATOR_CPP_MANAGERS_SRC_HDR
#define HEEL_GENERATOR_CPP_MANAGERS_SRC_HDR

#include <string>

#include <heel/generator_utils.hpp>
#include <heel/model_application.hpp>

namespace margot {
namespace heel {

// this function generates the struct that defines the high-level interface, including the monitors and goals.
cpp_source_content managers_cpp_content(application_model& app, const std::string& app_description);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_GENERATOR_CPP_MANAGERS_SRC_HDR