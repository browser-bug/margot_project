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
#include <memory>


#include "margot/circular_buffer.hpp"
#include "margot/statistics.hpp"
#include "margot/enums.hpp"

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
       * The idea is to use the statistical_t type for computing the average value
       * or the standard deviation value. However, if the stored elements have
       * higher precision, then we must use the latter type.
       */
      using statistical_type = decltype( statistical_t{} / T{} );


      /**
       * @brief Explicit defition of the value stored in the CircularBuffer
       *
       * @details
       * This type is used to by the methods whose find the minimum or the
       * maximum value between the observed ones.
       */
      using value_type = typename CircularBuffer<T>::value_type;


      /**
       * @brief Default constructor
       *
       * @param [in] size The maximum number of stored in the CircularBuffer
       */
      StatisticalProvider( const std::size_t size ): CircularBuffer<T>(size)
      {
        previous_average = statistical_type{0};
        previous_stddev = statistical_type{0};
        previous_min = value_type{0};
        previous_min = value_type{0};
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
      inline value_type max( void )
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
      inline value_type min( void )
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
      inline value_type compute_max( void )
      {
        assert(CircularBuffer< T >::buffer.size() > 0
               && "Attempt to get the maximum element from an empty buffer");

        if (max_computed_time < CircularBuffer< T >::last_change)
        {
          previous_max = margot::max(CircularBuffer< T >::buffer);
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
      inline value_type compute_min( void )
      {
        assert(CircularBuffer< T >::buffer.size() > 0
               && "Attempt to get the minimum element from an empty buffer");

        if (min_computed_time < CircularBuffer< T >::last_change)
        {
          previous_min = margot::min(CircularBuffer< T >::buffer);
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
      value_type previous_max;
      value_type previous_min;
  };


  /******************************************************************
   *  HELPER FUNCTIONS FOR A STATISTICAL PROVIDER POINTER
   ******************************************************************/


  /**
   * @brief Helper struct, to retrieve a statistical property
   *
   * @tparam T The type of the observed elements
   * @tparam dg The target DataFunctions
   * @tparam statistical_t The minimum type used to compute statistical values
   *
   * @see DataFunctions
   * @see StatisticalProvider
   *
   * @details
   * This struct represents a unified interface to extract a statistical property
   * over a StatisticalProvider. For example, it states the concept of "i am
   * interest on the average value between the observed data".
   * This struct takes advantage of partial specialization to select the correct
   * method from the ones exposed by the StatisticalProvider class. Since this class
   * represents the general case, you should never be able to use this struct.
   */
  template< typename T, DataFunctions df, typename statistical_t = float  >
  struct monitor_utils
  {

    /**
     * @brief The type of the target statistical property
     *
     * @details
     * Since this is the general case, this type is never used, therefore it is
     * arbitrarily set to int.
     */
    using value_type = int;

    /**
     * @brief Retrive the value of the target statistical property
     *
     * @param [in] buffer A shared pointer to the target StatisticalProvider
     *
     * @details
     * Since this struct represents the general case, you should never use this
     * method. To enforce this behavior, it always trigger an assertion to
     * terminate the process.
     */
    inline static value_type get( const std::shared_ptr< StatisticalProvider<T, statistical_t> >& buffer)
    {
      assert(false && "Error: unable to extract the required data function from a monitor");
      return value_type{};
    }

  };


  /**
   * @brief Specialization of the helper struct, to retrieve the average value
   *
   * @tparam T The type of the observed elements
   * @tparam statistical_t The minimum type used to compute statistical values
   *
   * @see op_utils
   */
  template< typename T, typename statistical_t >
  struct monitor_utils< T, DataFunctions::AVERAGE, statistical_t >
  {

    /**
     * @brief The type of the average value, which is equal to StatisticalProvider<T, statistical_t>::statistical_type
     */
    using value_type = typename StatisticalProvider<T, statistical_t>::statistical_type;

    /**
     * @brief Retrive the average value of target StatisticalProvider
     *
     * @param [in] buffer A shared pointer to the target StatisticalProvider
     *
     * @return The average value
     */
    inline static value_type get( const std::shared_ptr< StatisticalProvider<T, statistical_t> >& buffer)
    {
      return buffer->average();
    }

  };


  /**
   * @brief Specialization of the helper struct, to retrieve the average value
   *
   * @tparam T The type of the observed elements
   * @tparam statistical_t The minimum type used to compute statistical values
   *
   * @see op_utils
   */
  template< typename T, typename statistical_t >
  struct monitor_utils< T, DataFunctions::STANDARD_DEVATION, statistical_t >
  {

    /**
     * @brief The type of the standard devation value, which is equal to StatisticalProvider<T, statistical_t>::statistical_type
     */
    using value_type = typename StatisticalProvider<T, statistical_t>::statistical_type;

    /**
     * @brief Retrive the standard deviation value of target StatisticalProvider
     *
     * @param [in] buffer A shared pointer to the target StatisticalProvider
     *
     * @return The standard deviation value
     */
    inline static value_type get( const std::shared_ptr< StatisticalProvider<T, statistical_t> >& buffer)
    {
      return buffer->standard_deviation();
    }

  };


  /**
   * @brief Specialization of the helper struct, to retrieve the maximum element
   *
   * @tparam T The type of the observed elements
   * @tparam statistical_t The minimum type used to compute statistical values
   *
   * @see op_utils
   */
  template< typename T, typename statistical_t >
  struct monitor_utils< T, DataFunctions::MAXIMUM, statistical_t >
  {

    /**
     * @brief The type of the maximum value, which is equal to StatisticalProvider<T, statistical_t>::value_type
     */
    using value_type = typename StatisticalProvider<T, statistical_t>::value_type;

    /**
     * @brief Retrive the maximum element observed in the CircularBuffer
     *
     * @param [in] buffer A shared pointer to the target StatisticalProvider
     *
     * @return The value of the maximum element observed in the CircularBuffer
     */
    inline static value_type get( const std::shared_ptr< StatisticalProvider<T, statistical_t> >& buffer)
    {
      return buffer->max();
    }

  };


  /**
   * @brief Specialization of the helper struct, to retrieve the minumum element
   *
   * @tparam T The type of the observed elements
   * @tparam statistical_t The minimum type used to compute statistical values
   *
   * @see op_utils
   */
  template< typename T, typename statistical_t >
  struct monitor_utils< T, DataFunctions::MINIMUM, statistical_t >
  {

    /**
     * @brief The type of the maximum value, which is equal to StatisticalProvider<T, statistical_t>::value_type
     */
    using value_type = typename StatisticalProvider<T, statistical_t>::value_type;

    /**
     * @brief Retrive the maximum element observed in the CircularBuffer
     *
     * @param [in] buffer A shared pointer to the target StatisticalProvider
     *
     * @return The value of the maximum element observed in the CircularBuffer
     */
    inline static value_type get( const std::shared_ptr< StatisticalProvider<T, statistical_t> >& buffer)
    {
      return buffer->min();
    }

  };



}

#endif // MARGOT_STATISTICAL_PROVIDER_HDR
