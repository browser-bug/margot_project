/* core/field_adaptor.hpp
 * Copyright (C) 2017 Davide Gadioli
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

#ifndef MARGOT_FIELD_ADAPTOR_HDR
#define MARGOT_FIELD_ADAPTOR_HDR

#include <cstddef>
#include <memory>
#include <array>

#include "margot/operating_point.hpp"
#include "margot/enums.hpp"
#include "margot/monitor.hpp"
#include "margot/traits.hpp"
#include "margot/statistics.hpp"

namespace margot
{

  template< class OperatingPoint, typename coefficient_type = float >
  class FieldAdaptorInterface
  {
    public:
      using OperatingPointPtr = std::shared_ptr< OperatingPoint >;

      virtual void evaluate_error( const OperatingPointPtr& op ) = 0;

      virtual coefficient_type get_error_coefficient( void ) = 0;

      virtual ~FieldAdaptorInterface( void ) {}

  };


  template< class OperatingPoint,
            OperatingPointSegments target_segment,
            std::size_t target_field_index,
            DataFunctions df,
            std::size_t inertia,
            typename coefficient_type = float >
  class FieldAdaptor: public FieldAdaptorInterface<OperatingPoint, coefficient_type>
  {

      // statically check the template argument
      static_assert(traits::is_operating_point<OperatingPoint>::value,
                    "Error: the knowledge base handles object with is_operating_point trait");


    public:

      using op_upper_bound_extractor = op_utils<OperatingPoint, target_segment, BoundType::UPPER>;
      using op_lower_bound_extractor = op_utils<OperatingPoint, target_segment, BoundType::LOWER>;

      using OperatingPointPtr = std::shared_ptr< OperatingPoint >;


      template< class T, typename statistical_t >
      FieldAdaptor( const Monitor<T, statistical_t>& monitor ): next_element(0)
      {
        // initialize the array
        error_window.fill(static_cast<coefficient_type>(1));

        // build the lambda that computes the error coefficient.
        // It copies the monitor buffer shared pointer, in this
        // way we are sure that it uses a valid object to extract
        // the data function.
        auto buffer = monitor.get_buffer();
        compute = [buffer] ( const OperatingPointPtr & op, bool & coefficient_is_valid)
        {
          // try to extract a valid measure from the monitor
          const auto observed_value = monitor_utils<T, df, statistical_t>::get(buffer, coefficient_is_valid);

          // if the measure is not valid we cannot draw any conclusion on the coefficient
          if (!coefficient_is_valid)
          {
            return static_cast< coefficient_type >(1);
          }

          // otherwise, get the upper and lower bound from the current Operating Point
          // the upper bound is average + standard devation
          // the lower bound is average - standard deviation
          const auto expected_upper_bound = op_upper_bound_extractor::template get< target_field_index, 1>(op);
          const auto expected_lower_bound = op_lower_bound_extractor::template get< target_field_index, 1>(op);

          // check if the observed value is outside the expected range of values
          if ((observed_value > expected_upper_bound) || (observed_value < expected_lower_bound))
          {
            // retrieve the expected average value from the current Operating Point
            const auto expected_average_value = op_upper_bound_extractor::template get< target_field_index, 0>(op);

            // at this point the expected average value and the observed value
            // might be of integer type. We must promote them at least to float,
            // otherwise the error coefficient will have funny values.
            using division_type = decltype( decltype(expected_average_value) {}
            * decltype(observed_value) {} * float{} );

            // compute the coefficient error required to adapt the knowledge base
            const division_type error = static_cast<division_type>(expected_average_value)
                                        / static_cast<division_type>(observed_value);

            // after the division, downcasting is ok. For the error coefficient,
            // the float precision is ok.
            return static_cast< coefficient_type >(error);
          }
          else
          {
            // everything is going as planned
            return static_cast< coefficient_type >(1);
          }
        };
      }


      void evaluate_error(const OperatingPointPtr& op )
      {
        // get the value and the validity of the new error coefficient
        bool is_valid = false;
        const auto new_coefficient = compute(op, is_valid);

        // if the new coefficient is valid, put it in the buffer
        if (is_valid)
        {
          error_window[next_element++] = new_coefficient;

          if (next_element == inertia)
          {
            next_element = 0;
          }
        }
      }

      coefficient_type get_error_coefficient( void )
      {
        return average(error_window);
      }

    private:


      std::array<coefficient_type, inertia> error_window;


      std::function<coefficient_type(const OperatingPointPtr&, bool& )> compute;

      std::size_t next_element;

  };




}

#endif // MARGOT_FIELD_ADAPTOR_HDR
