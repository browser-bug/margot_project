/* agora/doe.hpp
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


#ifndef MARGOT_AGORA_DOE_HDR
#define MARGOT_AGORA_DOE_HDR

#include <string>
#include <vector>


namespace margot
{

  // common data structures for handling the configurations server side
  using configuration_t = std::string;
  using field_design_space_t = std::vector< std::string >;
  using design_space_t = std::vector< field_design_space_t >;
  using design_of_experiments_t = std::vector< configuration_t >;


  // enum with all the available structs
  enum class DoeStrategy : uint_fast8_t
  {
    FULL_FACTORIAL = 0
  };

  // generic struct that generates the configuarions
  template< DoeStrategy strategy >
  struct planner;

  // specialization of the struct for the full factorial
  template<>
  struct planner<DoeStrategy::FULL_FACTORIAL>
  {

    static void explode( design_of_experiments_t& doe_plan, design_space_t input_space, configuration_t&& previous_configuration = {} )
    {
      // check if there we are done wit the configuration
      if (input_space.empty())
      {
        // we have completed the current configuration
        // we need to pop the last coma and we are done
        previous_configuration.pop_back();
        doe_plan.emplace_back(previous_configuration);
        return;
      }

      // otherwise we have to recursively explore the solutions
      const auto this_level_design_space = input_space.back();
      input_space.pop_back();


      for ( const auto& current_value : this_level_design_space )
      {
        // append the current value for the configuration
        configuration_t current_configuration = current_value + ",";
        current_configuration.append(previous_configuration);

        // recursively go down in the exploration
        explode(doe_plan, input_space, std::move(current_configuration));
      }
    }


    static inline design_of_experiments_t generate( design_space_t input_space)
    {
      // this is the output design of experiments
      design_of_experiments_t doe_plan;

      // perform a full factorial doe over the design space
      explode(doe_plan, input_space);

      return doe_plan;
    }
  };



}

#endif // MARGOT_AGORA_DOE_HDR
