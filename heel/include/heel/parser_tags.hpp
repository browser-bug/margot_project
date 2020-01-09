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

#ifndef HEEL_PARSER_TAGS_HDR
#define HEEL_PARSER_TAGS_HDR

#include <string>

namespace margot {
namespace heel {

// this struct is used to enforce consistency in the tags names of the json format
struct tag {
  inline static const std::string version(void) { return "version"; }
  inline static const std::string agora(void) { return "agora"; }
  inline static const std::string url(void) { return "borker_url"; }
  inline static const std::string username(void) { return "broker_username"; }
  inline static const std::string password(void) { return "broker_password"; }
  inline static const std::string qos(void) { return "broker_qos"; }
  inline static const std::string broker_ca(void) { return "broker_ca"; }
  inline static const std::string client_cert(void) { return "client_cert"; }
  inline static const std::string client_key(void) { return "client_key"; }
  inline static const std::string doe_plugin(void) { return "doe_plugin"; }
  inline static const std::string clustering_plugin(void) { return "clustering_plugin"; }
  inline static const std::string doe_parameters(void) { return "doe_parameters"; }
  inline static const std::string blocks(void) { return "blocks"; }
  inline static const std::string clustering_parameters(void) { return "clustering_parameters"; }
  inline static const std::string name(void) { return "name"; }
  inline static const std::string features(void) { return "features"; }
  inline static const std::string feature_distance(void) { return "feature_distance"; }
  inline static const std::string type(void) { return "type"; }
  inline static const std::string comparison(void) { return "comparison"; }
  inline static const std::string knobs(void) { return "knobs"; }
  inline static const std::string values(void) { return "values"; }
  inline static const std::string range(void) { return "range"; }
  inline static const std::string metrics(void) { return "metrics"; }
  inline static const std::string distribution(void) { return "distribution"; }
  inline static const std::string prediction_plugin(void) { return "prediction_plugin"; }
  inline static const std::string prediction_params(void) { return "prediction_parameters"; }
  inline static const std::string observed_by(void) { return "observed_by"; }
  inline static const std::string reactive_inertia(void) { return "reactive_inertia"; }
  inline static const std::string monitors(void) { return "monitors"; }
  inline static const std::string log(void) { return "log"; }
  inline static const std::string constructor(void) { return "constructor"; }
  inline static const std::string start(void) { return "start"; }
  inline static const std::string stop(void) { return "stop"; }
  inline static const std::string states(void) { return "extra-functional_requirements"; }
  inline static const std::string maximize(void) { return "maximize"; }
  inline static const std::string minimize(void) { return "minimize"; }
  inline static const std::string geometric_mean(void) { return "geometric_mean"; }
  inline static const std::string linear_mean(void) { return "linear_mean"; }
  inline static const std::string constraints(void) { return "subject_to"; }
  inline static const std::string subject(void) { return "subject"; }
  inline static const std::string value(void) { return "value"; }
  inline static const std::string confidence(void) { return "confidence"; }
};

}  // namespace heel
}  // namespace margot

#endif  // HEEL_PARSER_TAGS_HDR