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