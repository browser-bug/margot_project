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
       * @brief Default constructor
       */
      Monitor( void )
      {
        buffer.reset();
      }


      /**
       * @brief Initalize the monitor
       *
       * @param [in] size The number of elements stored in the CircularBuffer
       */
      Monitor( const std::size_t size )
      {
        buffer.reset( new statistical_provider_type(size));
      }


      /**
       * @brief Insert a new value in the circular buffer
       *
       * @param [in] new_value The value of the new observation
       */
      inline void push( const T new_value )
      {
        assert(buffer && "Error: attempt to push a value in an empty monitor");
        buffer->push(new_value);
      }

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
        assert(buffer && "Error: attempt to retrieve the circular buffer of an empty monitor");
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
