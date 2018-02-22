/* agora/common_objects.hpp
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


#ifndef MARGOT_AGORA_COMMON_OBJECTS_HDR
#define MARGOT_AGORA_COMMON_OBJECTS_HDR


#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <algorithm>

#include "doe.hpp"

namespace margot
{
  struct message_t
  {
    std::string topic;
    std::string payload;
  };

  struct knob_t
  {
    std::string name;
    std::string type;
    field_design_space_t values;
  };

  struct metric_t
  {
    std::string name;
    std::string type;
    std::string prediction_method;
  };

  struct feature_t
  {
    std::string name;
    std::string type;
    field_design_space_t values;
  };

  struct sort_fields_operator
  {
    inline bool operator() (const knob_t& knob1, const knob_t& knob2) const
    {
      return (knob1.name < knob2.name);
    }
    inline bool operator() (const metric_t& metric1, const metric_t& metric2) const
    {
      return (metric1.name < metric2.name);
    }
    inline bool operator() (const feature_t& feature1, const feature_t& feature2) const
    {
      return (feature1.name < feature2.name);
    }
  };

  using application_knobs_t = std::vector<knob_t>;
  using application_features_t = std::vector<feature_t>;
  using application_metrics_t = std::vector<metric_t>;


  struct model_t
  {

    // this method creates a model given the information of the application
    template< DoeStrategy policy >
    void create( application_knobs_t& knobs, application_features_t& features, application_metrics_t& metrics)
    {
      // declare the design space used to create the application model
      design_space_t design_space;

      // sort the data structures in lexinographic way
      std::sort(knobs.begin(), knobs.end(), sort_fields_operator());
      std::sort(features.begin(), features.end(), sort_fields_operator());
      std::sort(metrics.begin(), metrics.end(), sort_fields_operator());

      // fill the name and type for each field
      for (const auto& knob : knobs)
      {
        fields_name.emplace_back("k_" + knob.name);
        fields_type.emplace_back(knob.type);
        design_space.emplace_back(knob.values);
      }

      for (const auto& feature : features)
      {
        fields_name.emplace_back("f_" + feature.name);
        fields_type.emplace_back(feature.type);
        design_space.emplace_back(feature.values);
      }

      for (const auto& metric : metrics)
      {
        fields_name.emplace_back("m_mean_" + metric.name);
        fields_type.emplace_back(metric.type);
        fields_name.emplace_back("m_std_" + metric.name);
        fields_type.emplace_back(metric.type);
      }

      // creates the plan according to the given design of experiments
      struct planner<policy> generator;
      model_data = generator(design_space);
    }

    // the number of fields in the configuration
    inline bool usable( void ) const
    {
      const auto number_of_theoretical_fields = fields_name.size();
      return (number_of_theoretical_fields == num_data_fields()) && (number_of_theoretical_fields > 0);
    }

    inline int num_data_fields( void ) const
    {
      return !model_data.empty() ? std::count(model_data[0].begin(), model_data[0].end(), ',') + 1 : 0;
    }

    inline void clear( void )
    {
      fields_name.clear();
      fields_type.clear();
      model_data.clear();
    }


    std::vector< std::string > fields_name;
    std::vector< std::string > fields_type;  // actually optional when loading from fs
    std::vector< std::string > model_data;
  };


  struct doe_t
  {
    // this method creates a doe given the information of the application
    template< DoeStrategy policy >
    void create( application_knobs_t& knobs, const int required_number_of_observations )
    {
      // declare the design space used to create the application model
      design_space_t design_space;

      // sort the data structures in lexinographic way
      std::sort(knobs.begin(), knobs.end(), sort_fields_operator());

      // fill the name and type for each field
      for (const auto& knob : knobs)
      {
        fields_name.emplace_back("k_" + knob.name);
        fields_type.emplace_back(knob.type);
        design_space.emplace_back(knob.values);
      }

      // add the last field for the counter
      fields_name.emplace_back("counter");
      fields_type.emplace_back("int");

      // creates the plan according to the given design of experiments
      struct planner<policy> generator;
      design_of_experiments_t plan = generator(design_space);

      // initialize the actual data structure
      for ( auto&& configuration : plan )
      {
        doe.emplace(configuration, required_number_of_observations);
      }
    }

    inline bool usable( void ) const
    {
      return fields_name.size() > 0;
    }

    inline void clear( void )
    {
      fields_name.clear();
      fields_type.clear();
      doe.clear();
      next_configuration = doe.end();
    }

    std::vector< std::string > fields_name;
    std::vector< std::string > fields_type;          // actually optional when loading from fs
    std::unordered_map< configuration_t, int > doe;
    std::unordered_map< configuration_t, int >::iterator next_configuration;
  };


  using cliet_name_t = std::string;
  using application_list_t = std::unordered_set< cliet_name_t >;
  using application_map_t = std::unordered_map< cliet_name_t, configuration_t >;




}

#endif // MARGOT_AGORA_COMMON_OBJECTS_HDR
