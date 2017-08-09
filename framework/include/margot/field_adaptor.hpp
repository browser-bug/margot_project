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


  /**
   * @brief Compute an error coefficient of the knowledge base
   *
   * @tparam OperatingPoint The type of the target Operating Point
   * @tparam coefficient_type The type of the generated error coefficient
   *
   * @details
   * This class computes an error coefficient on the average value of a field
   * of the Operating Point as the ration between the expected mean value of
   * the target field and mean value observed at run-time.
   * This class is virtual, which means that it is independet on the actual
   * field and method to compute this coefficient
   */
  template< class OperatingPoint, typename coefficient_type = float >
  class FieldAdaptor
  {

    public:


      /**
       * @brief Explicit defition of pointer to the Operating Point
       */
      using OperatingPointPtr = std::shared_ptr< OperatingPoint >;


      /**
       * @brief Compute the new coefficient error
       *
       * @param [in] op The pointer to the Operating Point used by the application
       *
       * @details
       * This method aims at evaluating the expected behavior of the current
       * Operating Point, relate it with runtime information to compute the
       * error coefficient.
       */
      virtual void evaluate_error( const OperatingPointPtr& op ) = 0;


      /**
       * @brief Retrive the current coefficient error
       *
       * @return The value of the actual coefficient error
       *
       * @details
       * This method only retrieves the value of the precomputed coefficient error.
       * For performance reason, it should be as lightweight as possible.
       */
      virtual coefficient_type get_error_coefficient( void ) = 0;


      /**
       * @brief Virtual destructor for the interface
       */
      virtual ~FieldAdaptor( void ) {}

  };


  /**
   * @brief The implementation of FieldAdaptor, standard deviation aware
   *
   * @tparam OperatingPoint The type of the target Operating Point
   * @tparam target_segment The value of the target segment of the Operating Point
   * @tparam target_field_index The index value of the target field in the target segment
   * @tparam df The target data function to extract from the monitor
   * @tparam inertia The size of the underlying circular buffer
   * @tparam coefficient_type The type of the generated error coefficient
   *
   * @details
   * This class implements two features to keep as stable as possible the adaptation of
   * the knowledge base.
   * On one hand, it takes into account the standard deviation of the target field. In
   * particular, if statistical property is between one sigma from the average, then
   * this error is considered noise. Therefore the error coefficient is set to 1.
   * On the other hand, it uses a circular buffer to average any spykes in the of the
   * error coefficient. But, higher inertia, slower reaction.
   */
  template< class OperatingPoint,
            OperatingPointSegments target_segment,
            std::size_t target_field_index,
            DataFunctions df,
            std::size_t inertia,
            typename coefficient_type = float >
  class OneSigmaAdaptor: public FieldAdaptor<OperatingPoint, coefficient_type>
  {

      // statically check the template argument
      static_assert(traits::is_operating_point<OperatingPoint>::value,
                    "Error: the knowledge base handles object with is_operating_point trait");

    public:

      /**
       * @brief aliasing of the util struct that extracts an upper bound of the target field
       */
      using op_upper_bound_extractor = op_utils<OperatingPoint, target_segment, BoundType::UPPER>;


      /**
       *  @brief aliasing of the util struct that extracts a lower bound of the target field
       */
      using op_lower_bound_extractor = op_utils<OperatingPoint, target_segment, BoundType::LOWER>;


      /**
       * @brief Explicit definition to a pointer of the Operating Point
       */
      using OperatingPointPtr = std::shared_ptr< OperatingPoint >;


      /**
       * @brief Defualt constructor of the class
       *
       * @tparam T The type of the elements stored in the monitor
       * @tparam statistical_t The minimum type used to compute statistical informations
       *
       * @param [in] monitor The monitor used to obtain runtime information
       *
       * @details
       * This class initialize all the internal attributes. In particular it fills
       * the circular buffer with the ones, which means that it assumes that the
       * application knowledge fits the execution environment.
       * Then it generates the function that actually computes the coefficient
       * error.
       */
      template< class T, typename statistical_t >
      OneSigmaAdaptor( const Monitor<T, statistical_t>& monitor )
        : average_coefficient_error(static_cast<coefficient_type>(1)),next_element(0)
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


      /**
       * @brief This method generates the coefficient error
       *
       * @param [in] op A pointer to the Operating Point actually used by the application
       *
       * @details
       * This method generates the coefficient error and, if it is valid, push it in
       * the circular buffer. Otherwise the value is discarded.
       * In the former case, it computes also the average value of the coefficient
       * error across the circular buffer.
       */
      void evaluate_error(const OperatingPointPtr& op )
      {
        // get the value and the validity of the new error coefficient
        bool is_valid = false;
        const auto new_coefficient = compute(op, is_valid);

        // if the new coefficient is valid, put it in the buffer
        if (is_valid)
        {

          // insert the new value in the circular buffer
          error_window[next_element++] = new_coefficient;
          if (next_element == inertia)
          {
            next_element = 0;
          }

          // compute the new average
          average_coefficient_error = average(error_window);

        }
      }


      /**
       * @brief Retrieve the coefficient error
       *
       * @return The value of the coefficient error
       */
      coefficient_type get_error_coefficient( void )
      {
        return average_coefficient_error;
      }

    private:


      /**
       * @brief The circular buffer, whose size is fixed at compile time
       */
      std::array<coefficient_type, inertia> error_window;


      /**
       * @brief The precomputed coefficient error average
       */
      coefficient_type average_coefficient_error;


      /**
       * @brief A pointer to the function that computes the coefficient error
       */
      std::function<coefficient_type(const OperatingPointPtr&, bool& )> compute;


      /**
       * @brief Utility attribute that holds the index to the element to be overwritten
       */
      std::size_t next_element;

  };

}

#endif // MARGOT_FIELD_ADAPTOR_HDR
