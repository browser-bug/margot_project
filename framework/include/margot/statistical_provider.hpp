/* core/statistical_provider.hpp
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

#include <chrono>
#include <cstddef>
#include <cassert>


#include "margot/circular_buffer.hpp"
#include "margot/statistics.hpp"

#ifndef MARGOT_STATISTICAL_PROVIDER_HDR
#define MARGOT_STATISTICAL_PROVIDER_HDR

namespace margot
{

  /**
   * @brief This class enhance a CircularBuffer, extracting statistical properties
   *
   * @tparam T The type of the stored elements in the CircularBuffer
   * @tparam statistical_t The minimum type used to compute the statistical properties
   *
   * @details
   * While a CircularBuffer buffer is about storing a sliding window of elements,
   * this class aims at extracting statistical properties over its elements.
   * It exploits memoization mechanism to avoud useless computations. In particular,
   * it compares the timestamp of the last change in the CircularBuffer with the
   * timestamp of the computed value to check if it is possible to use the previous
   * computed value.
   */
  template< typename T, typename statistical_t = float >
  class StatisticalProvider: public CircularBuffer< T >
  {
      // This class requires an arithmetic value
      static_assert(std::is_arithmetic<T>::value, "Error: in the statistical provider context, the template T must be an arithmetic type");

    public:

      /**
       * @brief Explicit definition of the statistical type
       *
       * @details
       * The idea is to use the statistical_t type. However, if the stored elements
       * have higher precision, then it must use the latter type.
       */
      using statistical_type = decltype( statistical_t{} / T{} );


      /**
       * @brief Default constructor
       *
       * @param [in] size The maximum number of stored in the CircularBuffer
       */
      StatisticalProvider( const std::size_t size ): CircularBuffer<T>(size)
      {
        previous_average = statistical_type{0};
        previous_stddev = statistical_type{0};
        previous_min = statistical_type{0};
        previous_min = statistical_type{0};
      }


      /**
       * @brief Retrive the average of the observation window
       *
       * @return The average value of the container
       *
       * @details
       * If the CircularBuffer is not changed since the last time that we have
       * computed the average, than we might use the previous value
       */
      inline statistical_type average( void )
      {
        std::lock_guard<std::mutex> lock(CircularBuffer< T >::buffer_mutex);
        return compute_average();
      }


      /**
       * @brief Retrive the standard deviation of the observation window
       *
       * @return The standard deviation value of the container
       *
       * @details
       * If the CircularBuffer is not changed since the last time that we have
       * computed the average, than we might use the previous value.
       * To compute the standard devation, we must compute also the average value.
       */
      inline statistical_type standard_deviation( void )
      {
        std::lock_guard<std::mutex> lock(CircularBuffer< T >::buffer_mutex);
        return compute_standard_deviation();
      }


      /**
       * @brief Retrive the maximum element of the CircularBuffer
       *
       * @return The value of the maximum element of the container
       *
       * @details
       * If the CircularBuffer is not changed since the last time that we have
       * computed the average, than we might use the previous value.
       * We have chosen to promote the type of the returned element to the
       * statistical_type, to have a uniform interface.
       */
      inline statistical_type max( void )
      {
        std::lock_guard<std::mutex> lock(CircularBuffer< T >::buffer_mutex);
        return compute_max();
      }


      /**
       * @brief Retrive the minimum element of the CircularBuffer
       *
       * @return The value of the minimum element of the container
       *
       * @details
       * If the CircularBuffer is not changed since the last time that we have
       * computed the average, than we might use the previous value.
       * We have chosen to promote the type of the returned element to the
       * statistical_type, to have a uniform interface.
       */
      inline statistical_type min( void )
      {
        std::lock_guard<std::mutex> lock(CircularBuffer< T >::buffer_mutex);
        return compute_min();
      }


    private:

      /**
       * @details
       * This methods compute the actual average of the container. For performance
       * reason we check that the container must hold at least one value only
       * if the macro NDEBUG is not defined.
       */
      inline statistical_type compute_average( void )
      {
        assert(CircularBuffer< T >::buffer.size() > 0
               && "Attempt to get the average from an empty buffer");

        if (average_computed_time < CircularBuffer< T >::last_change)
        {
          previous_average = margot::average< typename CircularBuffer<T>::container_type,
          statistical_type >(CircularBuffer< T >::buffer);
          average_computed_time = CircularBuffer<T>::last_change;
        }

        return previous_average;
      }


      /**
       * @details
       * This methods compute the actual standaed deviation value of the elements
       * in the container. For performance reason, we check that the
       * container must hold at least one value only if the macro NDEBUG is not defined.
       */
      inline statistical_type compute_standard_deviation( void )
      {
        assert(CircularBuffer< T >::buffer.size() > 0
               && "Attempt to get the standard deviation from an empty buffer");

        if (stddev_computed_time < CircularBuffer< T >::last_change)
        {
          previous_stddev = margot::stddev< typename CircularBuffer<T>::container_type,
          statistical_type >(CircularBuffer< T >::buffer, compute_average());
          stddev_computed_time = CircularBuffer<T>::last_change;
        }

        return previous_stddev;
      }


      /**
       * @details
       * This methods compute the actual value of the maximum element between the elements
       * in the container. For performance reason, we check that the
       * container must hold at least one value only if the macro NDEBUG is not defined.
       */
      inline statistical_type compute_max( void )
      {
        assert(CircularBuffer< T >::buffer.size() > 0
               && "Attempt to get the maximum element from an empty buffer");

        if (max_computed_time < CircularBuffer< T >::last_change)
        {
          previous_max = static_cast<statistical_type>(margot::max(CircularBuffer< T >::buffer));
          max_computed_time = CircularBuffer<T>::last_change;
        }

        return previous_max;
      }


      /**
       * @details
       * This methods compute the actual value of the minimum element between the elements
       * in the container. For performance reason, we check that the
       * container must hold at least one value only if the macro NDEBUG is not defined.
       */
      inline statistical_type compute_min( void )
      {
        assert(CircularBuffer< T >::buffer.size() > 0
               && "Attempt to get the minimum element from an empty buffer");

        if (min_computed_time < CircularBuffer< T >::last_change)
        {
          previous_min = static_cast<statistical_type>(margot::min(CircularBuffer< T >::buffer));
          min_computed_time = CircularBuffer<T>::last_change;
        }

        return previous_min;
      }


      // version of computed value
      typename CircularBuffer< T >::time_point_type average_computed_time;
      typename CircularBuffer< T >::time_point_type stddev_computed_time;
      typename CircularBuffer< T >::time_point_type max_computed_time;
      typename CircularBuffer< T >::time_point_type min_computed_time;

      // precomputed values
      statistical_type previous_average;
      statistical_type previous_stddev;
      statistical_type previous_max;
      statistical_type previous_min;
  };

}

#endif // MARGOT_STATISTICAL_PROVIDER_HDR
