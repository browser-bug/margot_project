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

#ifndef HEEL_MODEL_APPLICATION_HDR
#define HEEL_MODEL_APPLICATION_HDR

#include <string>
#include <vector>

#include <heel/model_block.hpp>

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