/* core/circular_buffer.hpp
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
#ifndef MARGOT_CIRCULAR_BUFFER_HDR
#define MARGOT_CIRCULAR_BUFFER_HDR

#include <vector>
#include <algorithm>
#include <chrono>
#include <cstddef>
#include <mutex>
#include <cassert>


namespace margot
{

  /**
   * @brief A circular buffer that stores the last n observations
   *
   * @tparam T The type of the stored elements
   *
   * @details
   * This class represents an observation windows with a sliding temporal frame,
   * depending on the number of observation.
   * All the publics method that access the container are protected with a mutex,
   * therfore this class is thread-safe.
   */
  template< typename T >
  class CircularBuffer
  {

    public:


      /**
       * @brief Explicit definition of the type of the stored elements
       */
      using value_type = T;


      /**
       * @brief The type of the container used by the circular buffer
       *
       * @details
       * For performance reason, we have choose to store all the elements in a
       * vector, using a index reference to keep track of the next element to
       * overwrite.
       */
      using container_type = std::vector<T>;


      /**
       * @brief Default constructor
       *
       * @param [in] size The maximum number of elements stored in the buffer
       */
      CircularBuffer( const std::size_t size):
        next_element(0), maximum_number_element(size)
      {
        buffer.reserve(size);
        last_change = std::chrono::steady_clock::now();
      }


      /******************************************************************
       *  METHODS TO ALTER THE CIRCULAR BUFFER
       ******************************************************************/


      /**
       * @brief Store a new element inside the circular buffer
       *
       * @details
       * The size of the container grows at runtime until we reach the maximum
       * number of elements. Afterward, each new element replace the last one.
       */
      void push( const T new_value)
      {
        std::lock_guard<std::mutex> lock(buffer_mutex);

        if (buffer.size() < maximum_number_element)
        {
          buffer.emplace_back(new_value);
        }
        else
        {
          buffer[next_element++] = new_value;

          if (next_element == maximum_number_element)
          {
            next_element = 0;
          }
        }

        last_change = std::chrono::steady_clock::now();
      }


      /**
       * @brief Clear the container from all the values
       *
       * @details
       * This method calls the clear method of the underlying container, therefore
       * it changes the size of the container
       */
      inline void clear( void )
      {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        next_element = 0;
        buffer.clear();
        last_change = std::chrono::steady_clock::now();
      }


      /******************************************************************
       *  UTILITY METHODS FOR THE CIRCULAR BUFFER
       ******************************************************************/

      /**
       * @brief Test whether the container is empty
       *
       * @return True, if the container is empty, i.e. it has no elements
       */
      inline bool empty( void ) const
      {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        return buffer.empty();
      }


      /**
       * @brief Test whether the container is full
       *
       * @return True, if the size of the container is equal to the maximum number of elements
       */
      inline bool full( void ) const
      {
        std::lock_guard<std::mutex> lock(buffer_mutex);
        return buffer.size() == maximum_number_element;
      }


      /**
       * @brief Retrieve the last element of the container
       *
       * @return The last element inserted in the container
       */
      inline T last( void ) const
      {
        std::lock_guard<std::mutex> lock(buffer_mutex);

        assert(buffer.size() > 0
               && "Attempt to get the last element from an empty buffer");

        // even in the case that buffer.size() < maximum_number_element, the last
        // element is still the back of the buffer
        return next_element == 0 ? buffer.back() : buffer[next_element - 1];
      }


    protected:


      /**
       * @brief Definition of the time point
       */
      using time_point_type = std::chrono::steady_clock::time_point;


      /**
       * @brief Retrieve a reference to the begin of the container
       *
       * @return A const interator to the beginning of the container
       */
      inline typename container_type::const_iterator cbegin( void ) const
      {
        return buffer.cbegin();
      }


      /**
       * @brief Retrieve a reference to the end of the container
       *
       * @return A const iterator to the ending of the container
       */
      inline typename container_type::const_iterator cend( void ) const
      {
        return buffer.cend();
      }


      /**
       * @brief Retrieve the number of observations
       *
       * @return The size of the container
       */
      inline std::size_t size( void ) const
      {
        return buffer.size();
      }

      /**
       * @brief The unfolded circular buffer
       */
      container_type buffer;

      /**
      * @brief The timestamp of the last modification
      */
      std::chrono::steady_clock::time_point last_change;

      /**
       * @brief The mutex used to protect the access to the buffer
       */
      mutable std::mutex buffer_mutex;


    private:


      /**
       * @brief The index of the next element to be overwritten
       */
      std::size_t next_element;

      /**
       * @brief The maximum number of elements in the container
       */
      const std::size_t maximum_number_element;

  };


}

#endif // MARGOT_CIRCULAR_BUFFER_HDR
