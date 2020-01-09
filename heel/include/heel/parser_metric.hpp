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

#ifndef HEEL_PARSER_METRIC_HDR
#define HEEL_PARSER_METRIC_HDR

#include <boost/property_tree/ptree.hpp>

#include <heel/model_metric.hpp>

namespace margot {
namespace heel {

void parse(metric_model& metric, const boost::property_tree::ptree& metric_node);

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_METRIC_HDR