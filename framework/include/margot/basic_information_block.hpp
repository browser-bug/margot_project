/* core/basic_information_block.hpp
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
#ifndef MARGOT_BASIC_INFORMATION_BLOCK_HDR
#define MARGOT_BASIC_INFORMATION_BLOCK_HDR

#include <type_traits>
#include <functional>

#include "margot/hash.hpp"
#include "margot/traits.hpp"


namespace margot
{

  /**
   * @brief The simple data field of an Operating Point segment
   *
   * @tparam T The type of the stored data
   *
   * @details
   * This struct represents the average value of a field of an
   * Operating Point segment.
   * It can represent either the vale software knob or the mean
   * value of a metric.
   *
   * @note
   * The type T must have a true value on the trait std::is_arithmetic
   */
  template< class T >
  struct Data
  {
    static_assert(std::is_arithmetic<T>::value, "Error: a Data template parameter must be of a arithmetic value");
    using value_type = T;

    Data( const T mean )
      : mean(mean) {}

    const T mean;
  };

  /**
   * @brief This struct enhance a Data struct with a standard deviation
   *
   * @tparam T The type of the stored mean
   *
   * @details
   * This struct enhance a Data struct with the information about the
   * standard deviation of its mean.
   * Usually, it represents a data feature or a metric of an Operating
   * Point.
   *
   * @note
   * The type T must have a true value on the trait std::is_arithmetic.
   * The of the standard deviation is always a floating point type, the
   * actual type depends is define as decltype(float{} / T{}) to deal
   * with the case when T is a floating point with greater precision
   * with respect to float.
   */
  template< class T >
  struct Distribution: public Data<T>
  {
    static_assert(std::is_arithmetic<T>::value, "Error: a Distribution template parameter must be of a arithmetic value");
    using statistics_type = decltype(float{} / T{});

    Distribution( const T value, const statistics_type standard_deviation = statistics_type{} )
      : Data<T>(value), standard_deviation(standard_deviation) {}

    const statistics_type standard_deviation;
  };



  /******************************************************************
   *  SPECIALIZED OPERATORS FOR THE PREVIOUS STRUCTS
   ******************************************************************/

  /**
   * @brief The == operator for Data objects
   */
  template< class T >
  inline bool operator==(const Data<T>& lhs, const Data<T>& rhs)
  {
    return lhs.mean == rhs.mean;
  }

  /**
   * @brief The != operator for Data objects
   */
  template< class T >
  inline bool operator!=(const Data<T>& lhs, const Data<T>& rhs)
  {
    return !(lhs.mean == rhs.mean);
  }

  /**
   * @brief The != operator for Distribution objects
   */
  template< class T >
  inline bool operator==(const Distribution<T>& lhs, const Distribution<T>& rhs)
  {
    return lhs.mean == rhs.mean;
  }

  /**
   * @brief The != operator for Distribution objects
   */
  template< class T >
  inline bool operator!=(const Distribution<T>& lhs, const Distribution<T>& rhs)
  {
    return !(lhs.mean == rhs.mean);
  }


  /******************************************************************
   *  SPECIALIZATION OF THE HASH STRUCT
   ******************************************************************/

  /**
   * @brief Partial specialization of hash struct for Data type
   *
   * @tparam T The type of the mean value of Data
   */
  template< class T >
  struct hash< Data<T> >
  {
    /**
     * @brief call operator that computes the hash value
     *
     * @param [in] datum the target Data object
     *
     * @details
     * Since the Data struct enforce the type T of being
     * an arithmetic one, we can rely on std::hash to
     * compute its value
     */
    std::size_t operator()( Data<T> const& datum ) const
    {
      return std::hash<T>()(datum.mean);
    }
  };

  /**
   * @brief Partial specialization of hash struct for Distribution type
   *
   * @tparam T The type of the mean value of Distribution
   */
  template< class T >
  struct hash< Distribution<T> >
  {
    /**
     * @brief call operator that computes the hash value
     *
     * @param [in] datum the target Distribution object
     *
     * @details
     * Since the Distribution struct enforce the type T of being
     * an arithmetic one, we can rely on std::hash to
     * compute its value
     */
    std::size_t operator()( Distribution<T> const& datum ) const
    {
      return std::hash<T>()(datum.mean);
    }
  };


  /******************************************************************
   *  SPECIALIZATION OF THE TRAITS STRUCTS
   ******************************************************************/

  namespace traits
  {

    template < class T >
    struct has_mean< Data<T> >
    {
      using value_type = typename Data<T>::value_type;
      static constexpr bool value = true;
    };

    template < class T >
    struct has_mean< Distribution<T> >
    {
      using value_type = typename Distribution<T>::value_type;
      static constexpr bool value = true;
    };

    template < class T >
    struct has_standard_deviation< Data<T> >
    {
      using value_type = float;
      static constexpr bool value = false;
    };

    template < class T >
    struct has_standard_deviation< Distribution<T> >
    {
      using value_type = typename Distribution<T>::statistics_type;
      static constexpr bool value = true;
    };

  }

}

#endif // MARGOT_BASIC_INFORMATION_BLOCK_HDR
