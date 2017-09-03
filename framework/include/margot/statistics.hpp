/* core/statistics.hpp
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

#ifndef MARGOT_STATISTICS_HDR
#define MARGOT_STATISTICS_HDR

#include <functional>
#include <algorithm>
#include <numeric>
#include <cstddef>
#include <cmath>
#include <memory>

namespace margot
{


  /**
   * @brief Compute the average of the elements in a container
   *
   * @tparam T The type of the container
   * @tparam statistics_t The minimum type of the computation
   *
   * @param [in] c The target container
   *
   * @return The average of the elements in the container
   *
   * @details
   * This function requires that T implements: (a) the cbegin method, (b) the cend
   * method, (c) the size method and (d) define a T::value_type type.
   * It uses type promotion, to promote the computation of the average, in case the
   * type of the stored elements in the container are bigger than statistics_t.
   */
  template< class T, class statistics_t = float>
  inline auto average( const T&  c )
  -> decltype(statistics_t {} / typename T::value_type{})
  {
    return std::accumulate(c.cbegin(), c.cend(), decltype(statistics_t{} / typename T::value_type{}) {})
    / static_cast < decltype(statistics_t{} / typename T::value_type{}) > (c.size() > 1 ? c.size() : 1);
  }


  /**
   * @brief Compute the standard deviation of the elements in a container
   *
   * @tparam T The type of the container
   * @tparam statistics_t The minimum type of the computation
   *
   * @param [in] c The target container
   * @param [in] average The average of the elements in the container
   *
   * @return The standard deviation of the elements in the container
   *
   * @details
   * This function requires that T implements: (a) the cbegin method, (b) the cend
   * method, (c) the size method and (d) define a T::value_type type.
   * It uses type promotion, to promote the computation of the standard deviation,
   * in case the type of the stored elements in the container are bigger than statistics_t.
   */
  template< class T, class statistics_t = float >
  inline auto stddev( const T& c, const decltype(statistics_t {} / typename T::value_type{}) average)
  -> decltype(statistics_t{} / typename T::value_type{})
  {
    return std::sqrt(
    std::accumulate(c.cbegin(), c.cend(), decltype(statistics_t{} / typename T::value_type{}) {},
    [average] (const decltype(statistics_t {} / typename T::value_type{}) & summ,
    const typename T::value_type & d)
    {
      return summ + ((static_cast < decltype(statistics_t{} / typename T::value_type{}) > (d) - average) *
                     (static_cast < decltype(statistics_t{} / typename T::value_type{}) > (d) - average));
    }
                            ) / static_cast < decltype(statistics_t{} / typename T::value_type{}) > (c.size() > 1 ? c.size() - 1 : 1)
           );
  }


  /**
   * @brief Find the maximum element of a container
   *
   * @tparam T The type of the container
   *
   * @param [in] c The target container
   *
   * @return The value of the maximum element in a container
   *
   * @details
   * This function requires that T implements: (a) the cbegin method, (b) the
   * cend method.
   *
   * @note
   * If the container is empty, this method has an undefined behavior
   */
  template< class T >
  inline typename T::value_type max( const T& c )
  {
    return *std::max_element(c.cbegin(), c.cend());
  }


  /**
   * @brief Find the minimum element of a container
   *
   * @tparam T The type of the container
   *
   * @param [in] c The target container
   *
   * @return The value of the minimum element in a container
   *
   * @details
   * This function requires that T implements: (a) the cbegin method, (b) the
   * cend method.
   *
   * @note
   * If the container is empty, this method has an undefined behavior
   */
  template< class T >
  inline typename T::value_type min( const T& c )
  {
    return *std::min_element(c.cbegin(), c.cend());
  }



}

#endif // MARGOT_STATISTICS_HDR
