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
     * This trait is meant to be used to specify if an object holds a mean value.
     * In the framework context, it is used to characterize basic information
     * blocks.
     * @see Data @see Distribution
     * To qualify for this trait, the object T must (a) have a public numerical attribute
     * which is called mean and (b) define a type named mean_type
     */
    template < class T >
    struct has_mean
    {
      /**
       * @brief The type of the mean
       *
       * @details
       * By default this type is meant to be as small as possible. A real object
       * that has the trait is able to define the correct type of the mean.
       * This dummy value does not force any type promotion in the remainder
       * of the framework.
       */
      using mean_type = int;

      /**
       * @brief By default, the object T doesn't have a mean
       *
       * @details
       * If an object has a mean, it must specialize this struct and explicitly set
       * the relative attribute to a true value.
       */
      static constexpr bool value = false;
    };


    /**
     * @brief Used to test whether an object T has a standard deviation
     *
     * @details
     * This trait is meant to be used to specify if an object holds a standard
     * deviation value.
     * In the framework context, it is used to characterize basic information
     * blocks.
     * @see Distribution
     * To qualify for this trait, the object T must (a) have a public numerical attribute
     * which is called standard_deviation and (b) define a type named mean_type
     */
    template < class T >
    struct has_standard_deviation
    {
      /**
       * @brief The type of the standard deviation
       *
       * @details
       * By default this type is meant to be as small as possible. A real object
       * that has the trait is able to define the correct type of the standard
       * deviation.
       * This dummy value does not force any type promotion in the remainder
       * of the framework.
       */
      using standard_deviation_type = int;

      /**
       * @brief By default, the object T doesn't have a standard deviation
       *
       * @details
       * If an object has a standard deviation, it must specialize this struct and
       * explicitly set the relative attribute to a true value.
       */
      static constexpr bool value = false;
    };


    /**
     * @brief Used to test wheter an object T is an Operating Point segment
     *
     * @see OperatingPointSegment
     *
     * @details
     * This trait is meant to be used to specify if an object implements the
     * functionality of an Operating Point segmant.
     * In the context of the framework, it is used for static checks in the
     * definition of an Operating Point @see OperatingPoint.
     */
    template < class T >
    struct is_operating_point_segment
    {

      /**
       * @brief By default, the object T is not an Operating Point segment
       *
       * @details
       * If an object might qulify for an Operating Point segment, it must specialize
       * this struct and explicitly set the relative attribute to a true value.
       */
      static constexpr bool value = false;
    };

  }

}



#endif // MARGOT_TRAITS_HDR
