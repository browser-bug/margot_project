/* core/monitor.hpp
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

#ifndef MARGOT_MONITOR_HPP
#define MARGOT_MONITOR_HPP

#include <memory>
#include <cstddef>
#include <cassert>


#include "margot/statistical_provider.hpp"


namespace margot
{

  /**
   * @brief The basic class of a monitor
   *
   * @tparam T The type of the stored elements
   * @tparam statistics_t The minimum type used to extract statistical properties
   *
   * @see StatisticalProvider
   * @see CircularBuffer
   *
   * @details
   * This class represents a runtime monitor of the mARGOt framework. It stores
   * the elements using a smart pointer. Therefore, copying or moving this object
   * does not require a lot effort.
   * The basic idea is that this class implements all the required methods to
   * integrate with the Application-Specific RunTime Manger. A monitor should
   * implement only the function that gather a measure and push the observed
   * value in the CircularBuffer.
   * However, this class is meant to be used as a stand alone monitor, in fact
   * it forward several utility methods and it is possible to extract statistical
   * properties over the observations.
   *
   * @note
   * If a statistical properties is extracted from an empty monitor, then the
   * implementation returns the default value of the the type of the target
   * statistical property
   */
  template< typename T, typename statistics_t = float >
  class Monitor
  {


      /**
       * @brief Explicit definition of circular buffer
       */
      using statistical_provider_type = StatisticalProvider<T, statistics_t>;


    public:


      /**
       * @brief Explicit definition of a pointer to the circular buffer
       */
      using monitor_ptr_type = std::shared_ptr< statistical_provider_type >;

      /**
       * @brief Explicit definition of the statistical type
       *
       * @details
       * The idea is to use the statistical_t type. However, if the stored elements
       * have higher precision, then it must use the latter type.
       */
      using statistical_type = typename statistical_provider_type::statistical_type;


      /**
       * @brief Constructor of the monitor
       *
       * @param [in] size The number of elements stored in the CircularBuffer
       *
       * @details
       * This method allocates the CircularBuffer. By default, its size is 1.
       */
      Monitor( const std::size_t size = 1)
      {
        buffer.reset( new statistical_provider_type(size));
      }


      /******************************************************************
       *  FORWARD METHODS TO ALTER THE CIRCULAR BUFFER
       ******************************************************************/


      /**
       * @brief Insert a new value in the circular buffer
       *
       * @param [in] new_value The value of the new observation
       */
      inline void push( const T new_value )
      {
        buffer->push(new_value);
      }


      /**
       * @brief Clear the monitor from all the observed values
       *
       * @details
       * This method calls the clear method of the underlying container, therefore
       * it changes the size of the monitor
       */
      inline void clear( void )
      {
        buffer->clear();
      }


      /******************************************************************
       *  FORWARD OF UTILITY METHODS
       ******************************************************************/


      /**
       * @brief Test whether the monitor is empty
       *
       * @return True, if the monitor is empty, i.e. it has no elements
       */
      inline bool empty( void ) const
      {
        return buffer->empty();
      }


      /**
       * @brief Test whether the monitor is full
       *
       * @return True, if the size of the monitor is equal to the maximum number of elements
       */
      inline bool full( void ) const
      {
        return buffer->full();
      }


      /**
       * @brief Retrieve the last element of the monitor
       *
       * @return The last element observed by the monitor
       */
      inline T last( void ) const
      {
        return buffer->last();
      }


      /**
       * @brief Retrieve the number of observations
       *
       * @return The size of the buffer
       */
      inline std::size_t size( void ) const
      {
        return buffer->size();
      }


      /******************************************************************
       *  FORWARD OF THE METHODS TO EXTRACT STATISTICAL INFORMATION
       ******************************************************************/


      /**
       * @brief Retrive the average of the observation window
       *
       * @return The average value of the monitor
       */
      inline statistical_type average( void )
      {
        return buffer->average();
      }


      /**
       * @brief Retrive the standard deviation of the observation window
       *
       * @return The standard deviation value of the monitor
       */
      inline statistical_type standard_deviation( void )
      {
        return buffer->standard_deviation();
      }


      /**
       * @brief Retrive the maximum element of the observation window
       *
       * @return The value of the maximum element of the monitor
       */
      inline T max( void )
      {
        return buffer->max();
      }


      /**
       * @brief Retrive the minimum element of the CircularBuffer
       *
       * @return The value of the minimum element of the monitor
       */
      inline T min( void )
      {
        return buffer->min();
      }


      /******************************************************************
       *  INTEGRATION METHODS WITH THE FRAMEWORK
       ******************************************************************/


      /**
       * @brief Retrieve a pointer to the CircularBuffer
       *
       * @return A shared pointer to the CircularBuffer
       *
       * @see StatisticalProvider
       *
       * @note
       * This method should be used only by the framework internal objects.
       */
      inline monitor_ptr_type get_buffer( void ) const
      {
        return buffer;
      }


    private:


      /**
       * @brief The pointer to the CircularBuffer
       */
      monitor_ptr_type buffer;
  };

}

#endif // MARGOT_MONITOR_HPP
