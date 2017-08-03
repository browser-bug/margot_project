/* core/traits.hpp
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
#ifndef MARGOT_TRAITS_HDR
#define MARGOT_TRAITS_HDR

namespace margot
{

  /**
   * @brief Common namespace for all trait-specific code
   *
   * @details
   * This namespace is used to gather all the traits used by
   * the margot framwork in order to choose at compile time
   * the most suitable implementation of a function.
   */
  namespace traits
  {

    /**
     * @brief Used to test whether an object T has a mean
     *
     * @details
     * This trait is meant to be used to specify if an object
     * holds a mean value.
     * Typically is used to characterize basic information blocks.
     * By default the struct has a false value. If the given object
     * has a mean value, the developer must specialize the struct
     * with the type of the given object
     */
    template < class T >
    struct has_mean
    {
      using value_type = int;
      static constexpr bool value = false;
    };


    /**
     * @brief Used to test whether an object T has a standard deviation
     *
     * @details
     * This trait is meant to be used to specify if an object
     * holds a standard deviation.
     * Typically is used to characterize basic information blocks.
     * By default the struct has a false value. If the given object
     * has a standard deviation, the developer must specialize the
     * struct with the type of the given object
     */
    template < class T >
    struct has_standard_deviation
    {
      using value_type = int;
      static constexpr bool value = false;
    };

    /**
     * @brief Used to test wheter an object T is an Operating Point segment
     *
     * @details
     * This trait is meant to be used to specify if an object implements the
     * functionality of an Operating Point segmant.
     * By default the struct has a false value. If the given object
     * has a standard deviation, the developer must specialize the
     * struct with the type of the given object
     */
    template < class T >
    struct is_operating_point_segment
    {
      static constexpr bool value = false;
    };

  }

}



#endif // MARGOT_TRAITS_HDR
