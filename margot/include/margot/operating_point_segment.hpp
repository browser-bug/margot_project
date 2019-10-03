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

#include <array>
#include <cstddef>
#include <type_traits>

#include "margot/basic_information_block.hpp"
#include "margot/hash.hpp"
#include "margot/traits.hpp"

namespace margot {

/**
 * @brief This class represents a whole segment of the Operating Point
 *
 * @see OperatingPoint
 *
 * @tparam number_of_fields The number of elements in the segment
 * @tparam DataType The type of the elements of the field
 *
 * @details
 * This class represents a segment of an Operating Point, such as
 * the list of software knobs or the list of metrics of interest.
 *
 * This class is an enhanced std::array that takes into account that
 * the elements of such container are object with a mean and, optionally,
 * a standard deviation.
 *
 * The content of the array is specified in the constructor and it
 * is constant, for performance reasons.
 *
 * @note
 * The DataType object must implements the trait has_mean.
 * The size of field must be greater than zero.
 *
 */
template <std::size_t number_of_fields, class DataType>
class OperatingPointSegment {
  // static check for the properties of the templates
  static_assert(traits::has_mean<DataType>::value,
                "Error: an Operating Point segment point must provide at least its mean value");
  static_assert(number_of_fields > 0, "Error: An Operating Point segment must hold at least one Data value");

  /**
   * @brief The type of the container used to define the segment
   */
  using container_type = std::array<DataType, number_of_fields>;

  /**
   * @brief The definition of the variable that contains the actual values of the segment
   */
  const container_type fields;

  /**
   * @brief The pre-computed hash of the segment
   */
  const std::size_t hash;

 public:
  /**
   * @brief Definition of type of elements stored in the underlying container
   */
  using value_type = DataType;

  /**
   * @brief The typedef of the type of the mean value
   *
   * @details
   * Since DataType implements the has_mean trait, this value is the one defined
   * in DataType. It might be integer or a floating point.
   */
  using mean_type = typename traits::has_mean<DataType>::mean_type;

  /**
   * @brief The typedef of the type of the standard deviation value
   *
   * @details
   * If DataType implements the has_standard_deviation trait, then this value is
   * at least a floating point. Otherwise is forced to integer.
   */
  using standard_deviation_type = typename traits::has_standard_deviation<DataType>::standard_deviation_type;

  /**
   * @brief The number of fields that compose this segment of the Operating Point
   */
  static constexpr std::size_t size = number_of_fields;

  /**
   * @brief Default constructor of the segment
   *
   * @tparam T Variadic template argument that holds the type of values initialize the fields
   *
   * @param [in] args Variadic argument that initialize the elements of the field
   *
   * @details
   * This constructor initialize directly the elements of the container and computes
   * its hash value.
   * The type of the arguments (T) might be different with respect to the type of the
   * mean (DataType::mean_type). However, the compiler must know how to convert T in
   * DataType::value_type. Otherwise it will raise an error.
   * To compute the hash value, it takes advantage of the helper function
   * compute_hash_fixed_size_object.
   */
  template <class... T>
  OperatingPointSegment(T... args)
      : fields({{args...}}), hash(compute_hash_fixed_size_object<container_type, number_of_fields>(fields)) {}

  /**
   * @brief Get the mean value of a given element of the field
   *
   * @tparam index The index of the target element
   *
   * @return The mean value of the element
   */
  template <std::size_t index>
  inline auto get_mean(void) const -> typename traits::has_mean<DataType>::mean_type {
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
   * is used only if traits::has_standard_deviation of DataType is true.
   * The return type of this method is DataType::standard_deviation_type.
   */
  template <std::size_t index, class T = DataType>
  inline typename std::enable_if<traits::has_standard_deviation<T>::value,
                                 typename traits::has_standard_deviation<T>::standard_deviation_type>::type
  get_standard_deviation(void) const {
    return std::get<index>(fields).standard_deviation;
  }

  /**
   * @brief Get the standard deviation of a given element of the field
   *
   * @tparam index The index of the target element
   *
   * @return The value Zero as a constant known at compile time
   *
   * @note
   * This method takes advantage of the SFINAE approach, in particular it
   * is used only if traits::has_standard_deviation of DataType is false.
   * In this case the type of the return is an integer.
   */
  template <std::size_t index, class T = DataType>
  inline constexpr
      typename std::enable_if<!traits::has_standard_deviation<T>::value,
                              typename traits::has_standard_deviation<T>::standard_deviation_type>::type
      get_standard_deviation(void) const {
    return static_cast<typename traits::has_standard_deviation<T>::standard_deviation_type>(0);
  }

  /**
   * @brief Get the computed hash value of the segment
   *
   * @return The computed hash value of the whole segment
   *
   * @details
   * The hash value is computed in the constructor. Therefore the complexity
   * of this function is O(1).
   */
  inline std::size_t get_hash(void) const { return hash; }

  /**
   * @brief Declaration of the friend operator ==
   */
  template <std::size_t nf, class dt>
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
template <std::size_t number_of_fields, class DataType>
inline bool operator==(const OperatingPointSegment<number_of_fields, DataType>& lhs,
                       const OperatingPointSegment<number_of_fields, DataType>& rhs) {
  // first check the hashes
  if (lhs.get_hash() != rhs.get_hash()) {
    return false;
  }

  // otherwise compare it elements-wise
  return lhs.fields == rhs.fields;
}

/**
 * @brief Implement the != operator between two segments
 *
 * @tparam number_of_fields The number of elements in the segment
 * @tparam DataType The type of the elements of the field
 *
 * @return the negation of the value returned by the operator ==
 */
template <std::size_t number_of_fields, class DataType>
inline bool operator!=(const OperatingPointSegment<number_of_fields, DataType>& lhs,
                       const OperatingPointSegment<number_of_fields, DataType>& rhs) {
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
template <std::size_t number_of_fields, class DataType>
struct hash<OperatingPointSegment<number_of_fields, DataType> > {
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
  std::size_t operator()(OperatingPointSegment<number_of_fields, DataType> const& segment) const {
    return segment.get_hash();
  }
};

/******************************************************************
 *  SPECIALIZATION OF THE TRAITS STRUCTS
 ******************************************************************/

namespace traits {

/**
 * @brief Partial specialization of the is_operating_point_segment trait for OperatingPointSegment objects
 *
 * @see is_operating_point_segment
 */
template <std::size_t number_of_fields, class DataType>
struct is_operating_point_segment<OperatingPointSegment<number_of_fields, DataType> > {
  /**
   * @brief State that the OperatingPointSegment object implements the is_operating_point_segment traits
   */
  static constexpr bool value = true;
};

}  // namespace traits

}  // namespace margot

#endif  // MARGOT_OPERATING_POINT_SEGMENT_HDR
