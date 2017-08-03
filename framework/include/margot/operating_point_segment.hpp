/* core/operating_point_segment.hpp
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
#ifndef MARGOT_OPERATING_POINT_SEGMENT_HDR
#define MARGOT_OPERATING_POINT_SEGMENT_HDR

#include <cstddef>
#include <type_traits>
#include <array>


#include "margot/basic_information_block.hpp"
#include "margot/traits.hpp"
#include "margot/hash.hpp"



namespace margot
{

  /**
   * @brief This class represents a whole segment of Operating Point
   *
   * @tparam number_of_fields The number of elements in the segment
   * @tparam DataType The type of the elements of the field
   *
   * @details
   * This class represents a segment of an Operating Point, such as
   * the list of software knobs or the list of metrics of interest.
   *
   * This class is an enhanced std::array that takes into account that
   * the elements of such container are object with a mean and maybe
   * also a standard deviation.
   *
   * The content of the array is specified in the constructor and it
   * is constant, for performance reasons.
   *
   * @note
   * The DataType object must return true on traits::has_mean
   * The DataType must define its type with value_type
   * The size of field must be greater than zero
   *
   */
  template< std::size_t number_of_fields, class DataType >
  class OperatingPointSegment
  {
      // static check for the properties of the templates
      static_assert(traits::has_mean<DataType>::value, "Error: an Operating Point segment point must provide at least its mean value");
      static_assert(number_of_fields > 0, "Error: An Operating Point segment must hold at least one Data value");

      // the container used to store the informations
      using container_type =  std::array< DataType, number_of_fields >;


      /**
       * @brief The array that contains the actual values of the field
       */
      const container_type fields;

      /**
       * @brief The computed hash of the fields
       */
      const std::size_t hash;

    public:

      using mean_value_type = typename traits::has_mean<DataType>::value_type;
      using standard_deviation_value_type = typename traits::has_standard_deviation<DataType>::value_type;

      static constexpr std::size_t size = number_of_fields;

      /**
       * @brief Default constructor of the segment
       *
       * @tparam T Variadic template argument to with the types of parameters
       *
       * @param [in] args Variadic argument that initialize the elements of the fields
       *
       * @details
       * This constructor initialize directly the elements of the container and computes
       * its hash value.
       * The type of the arguments might be different, however the compiler must know how
       * to convert them in the type of DataType::value_type.
       * To compute the hash value, it takes advantage of the helper function
       * compute_hash_fixed_size_object.
       */
      template< class... T >
      OperatingPointSegment( T... args )
        : fields(
      {
        {
          args...
        }
      }), hash(compute_hash_fixed_size_object< container_type, number_of_fields >(fields))
      {

      }

      /**
       * @brief Get the mean value of a given element of the field
       *
       * @tparam index The index of the target element
       *
       * @return The mean value of the element
       */
      template< std::size_t index >
      inline auto get_mean( void ) const -> typename traits::has_mean< DataType >::value_type
      {
        return std::get<index>(fields).mean;
      }

      /**
       * @brief Get the standard deviation of a given element of the field
       *
       * @tparam index The index of the target element
       *
       * @return The standard deviation of the element
       *
       * @note
       * This method takes advantage of the SFINAE approach, in particular it
       * is used only if traits::has_mean of DataType is true
       */
      template< std::size_t index, class T = DataType >
      inline typename std::enable_if< traits::has_standard_deviation<T>::value,
             typename  traits::has_standard_deviation<T>::value_type >::type get_standard_deviation( void ) const
      {
        return std::get<index>(fields).standard_deviation;
      }

      /**
       * @brief Get the standard deviation of a given element of the field
       *
       * @tparam index The index of the target element
       *
       * @return The value Zero
       *
       * @note
       * This method takes advantage of the SFINAE approach, in particular it
       * is used only if traits::has_mean of DataType is false
       */
      template< std::size_t index, class T = DataType >
      inline constexpr typename std::enable_if < !traits::has_standard_deviation<T>::value,
             typename  traits::has_standard_deviation<T>::value_type >::type get_standard_deviation( void ) const
      {
        return static_cast<typename  traits::has_standard_deviation<T>::value_type>(0);
      }

      /**
       * @brief Get the computed hash value of the segment
       *
       * @return The computed hash value of the whole segment
       */
      inline std::size_t get_hash( void ) const
      {
        return hash;
      }

      template< std::size_t nf, class dt>
      friend inline bool operator==(const OperatingPointSegment<nf, dt>& lhs,
                                    const OperatingPointSegment<nf, dt>& rhs);
  };



  /******************************************************************
   *  SPECIALIZED OPERATORS FOR THE PREVIOUS STRUCTS
   ******************************************************************/


  /**
   * @brief Implement the == operator between two segments
   *
   * @tparam number_of_fields The number of elements in the segment
   * @tparam DataType The type of the elements of the fiel
   *
   * @return the value of the operator == between two std::array
   *
   * @details
   * For performance reasons, it first compares the hash value of the two
   * segments. The element-wise comparison is performed only if needed
   */
  template< std::size_t number_of_fields, class DataType >
  inline bool operator==(const OperatingPointSegment<number_of_fields, DataType>& lhs,
                         const OperatingPointSegment<number_of_fields, DataType>& rhs)
  {
    // first check the hashes
    if (lhs.get_hash() != rhs.get_hash())
    {
      return false;
    }

    // otherwise compare it elements-wise
    return lhs.fields == rhs.fields;
  }

  /**
   * @brief Implement the != operator between two segments
   *
   * @tparam number_of_fields The number of elements in the segment
   * @tparam DataType The type of the elements of the fiel
   *
   * @return the negation of the value returned by the operator ==
   */
  template< std::size_t number_of_fields, class DataType >
  inline bool operator!=(const OperatingPointSegment<number_of_fields, DataType>& lhs,
                         const OperatingPointSegment<number_of_fields, DataType>& rhs)
  {
    return !(lhs == rhs);
  }


  /******************************************************************
   *  SPECIALIZATION OF THE HASH STRUCT
   ******************************************************************/

  /**
   * @brief Partial specialization of hash struct for Data type
   *
   * @tparam number_of_fields The number of elements in the segment
   * @tparam DataType The type of the elements of the fiel
   */
  template< std::size_t number_of_fields, class DataType>
  struct hash< OperatingPointSegment< number_of_fields, DataType > >
  {
    /**
     * @brief call operator that computes the hash value
     *
     * @param [in] segment the target segment
     *
     * @details
     * Since the elements of a segment are constant and defined in the
     * constructor, this method retrieve the precomputed value
     * without computing it again.
     */
    std::size_t operator()( OperatingPointSegment< number_of_fields, DataType > const& segment ) const
    {
      return segment.get_hash();
    }
  };


  /******************************************************************
   *  SPECIALIZATION OF THE TRAITS STRUCTS
   ******************************************************************/

  namespace traits
  {

    template< std::size_t number_of_fields, class DataType >
    struct is_operating_point_segment< OperatingPointSegment<number_of_fields, DataType > >
    {
      static constexpr bool value = true;
    };

  }

}

#endif // MARGOT_OPERATING_POINT_SEGMENT_HDR
