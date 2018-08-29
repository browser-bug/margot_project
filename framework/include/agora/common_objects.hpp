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
#include <sstream>
#include <iostream>

#include "agora/doe.hpp"

namespace agora
{
  struct message_t
  {
    std::string topic;
    std::string payload;
  };

  struct knob_t
  {
    inline void set( const std::string description )
    {
      std::stringstream stream(description);
      stream >> name;         // get the name
      stream >> type;         // get the type
      std::string value = {};

      while (std::getline(stream, value, ' ')) // get the values
        if (!value.empty())
        {
          values.emplace_back(value);
        }
    }
    std::string name;
    std::string type;
    field_design_space_t values;
  };

  struct metric_t
  {
    inline void set( const std::string description )
    {
      std::stringstream stream(description);
      stream >> name;              // get the name
      stream >> type;              // get the type
      stream >> prediction_method; // get the prediction method
    }
    std::string name;
    std::string type;
    std::string prediction_method;
  };

  struct feature_t
  {
    inline void set( const std::string description )
    {
      std::stringstream stream(description);
      stream >> name;         // get the name
      stream >> type;         // get the type
      std::string value = {};

      while (std::getline(stream, value, ' ')) // get the values
        if (!value.empty())
        {
          values.emplace_back(value);
        }
    }
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


  struct application_description_t
  {
    application_description_t( void ) {}

    application_description_t( const std::string& application_name ): application_name(application_name) {}

    application_description_t(const std::string& application_name, application_knobs_t k, application_features_t f, application_metrics_t m)
      : application_name(application_name), knobs(k), features(f), metrics(m)
    {
      std::sort(knobs.begin(), knobs.end(), sort_fields_operator());
      std::sort(features.begin(), features.end(), sort_fields_operator());
      std::sort(metrics.begin(), metrics.end(), sort_fields_operator());
    }

    template< DoeStrategy policy >
    design_of_experiments_t get_design_experiement( const bool with_features ) const
    {
      design_space_t design_space;

      for ( const auto& knob : knobs )
      {
        design_space.emplace_back(knob.values);
      }

      if (with_features)
      {
        for ( const auto& feature : features )
        {
          design_space.emplace_back(feature.values);
        }
      }

      // return the doe
      return planner<policy>::generate(design_space);
    }

    inline void clear( void )
    {
      application_name.clear();
      number_point_per_dimension.clear();
      number_observations_per_point.clear();
      doe_name.clear();
      minimum_distance.clear();
      knobs.clear();
      features.clear();
      metrics.clear();
    }

    std::string application_name;
    std::string number_point_per_dimension;
    std::string number_observations_per_point;
    std::string doe_name;
    std::string minimum_distance;
    application_knobs_t knobs;
    application_features_t features;
    application_metrics_t metrics;
  };


  struct model_t
  {

    // this method creates a model given the information of the application
    void create( const application_description_t& description )
    {
      knowledge = description.get_design_experiement<DoeStrategy::FULL_FACTORIAL>(true);
    }

    inline int column_size( void ) const
    {
      return static_cast<int>(knowledge.empty() ? 0 : std::count(knowledge.front().begin(), knowledge.front().end(), ',') + 1);
    }

    std::string join( const application_description_t& description ) const
    {
      std::string result = "";
      const int offset_knobs_separator = description.knobs.size() - 1;
      const int offset_features_separator = description.knobs.size() + description.features.size() - 1;

      for ( auto entry : knowledge) // we want a copy of the entry
      {
        std::size_t offset = 0;
        int counter = 0;

        do
        {
          offset = entry.find_first_of(',', offset + 1);

          if (counter == offset_knobs_separator && offset != std::string::npos)
          {
            entry[offset] = ' ';
          }

          if (counter == offset_features_separator && offset != std::string::npos)
          {
            entry[offset] = ' ';
            break;
          }

          ++counter;
        }
        while ( offset != std::string::npos);

        result.append(entry + "@");
      }

      // remove the last separator
      result.pop_back();

      return result;
    }

    inline void clear( void )
    {
      knowledge.clear();
    }

    std::vector< std::string > knowledge;
  };


  struct doe_t
  {
    // this method creates a doe given the information of the application and doe strategy
    template< DoeStrategy policy >
    void create( const application_description_t& description, const int required_number_of_observations )
    {
      // declare the design space used to create the application model
      design_of_experiments_t doe = description.get_design_experiement<policy>(false);

      // initialize the actual data structure
      for ( auto configuration : doe )
      {
        required_explorations.emplace(configuration, required_number_of_observations);
      }

      // set the correct value for the doe iterator
      next_configuration = required_explorations.begin();
    }

    inline void clear( void )
    {
      required_explorations.clear();
      next_configuration = required_explorations.end();
    }

    std::unordered_map< configuration_t, int > required_explorations;
    std::unordered_map< configuration_t, int >::iterator next_configuration;
  };


  using cliet_name_t = std::string;
  using application_list_t = std::unordered_set< cliet_name_t >;
  using application_map_t = std::unordered_map< cliet_name_t, configuration_t >;




}

#endif // MARGOT_AGORA_COMMON_OBJECTS_HDR
