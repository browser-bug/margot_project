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
   * @brief The simple data field of an Operating Point segment.
   *
   * @see OperatingPointSegment
   *
   * @tparam T The type of the stored data, i.e. the mean
   *
   * @details
   * This struct represents the mean value of a field of an Operating Point segment.
   * It can represent either a software knobs or the mean value of a metric which
   * might represent as a constant value at runtime.
   *
   * @note
   * The type T must be arithmetic, i.e. have a true value on the trait std::is_arithmetic
   */
  template< class T >
  struct Data
  {
    static_assert(std::is_arithmetic<T>::value, "Error: in Data context, the template T must be an arithmetic type");

    /**
     * @brief The type of the mean value
     */
    using mean_type = T;

    /**
     * @brief Default constructor which initialize the mean value
     *
     * @param [in] mean The actual value of the mean
     */
    Data( const T mean )
      : mean(mean) {}

    /**
     * @brief A constant that express the mean value
     */
    const T mean;
  };

  /**
   * @brief This struct enhance a Data with a standard deviation
   *
   * @tparam T The type of the mean
   *
   * @details
   * This struct represents a distribution, therefore it enhances a Data struct
   * with the information about the standard deviation of the mean.
   * Usually, it represents a data feature or a metric of an Operating
   * Point.
   *
   * @note
   * The type T must be arithmetic, i.e. have a true value on the trait std::is_arithmetic.
   */
  template< class T >
  struct Distribution: public Data<T>
  {

    /**
     * @brief The type of the standard deviation
     *
     * @details
     * The type of the standard deviation is always a floating point type. The actual type
     * type is defined as decltype(float{} / T{}) to force a type promotion, according to T.
     */
    using standard_deviation_type = decltype(float{} / T{});

    /**
     * @brief Default constructor which initialize the fields of the Distribution
     *
     * @param [in] value The actual value of the mean
     * @param [in] standard_deviation The actual value of the standard deviation
     */
    Distribution( const T value, const standard_deviation_type standard_deviation = standard_deviation_type{} )
      : Data<T>(value), standard_deviation(standard_deviation) {}

    /**
     * @brief A constant which express the standard deviation
     */
    const standard_deviation_type standard_deviation;
  };



  /******************************************************************
   *  SPECIALIZED OPERATORS FOR THE PREVIOUS STRUCTS
   ******************************************************************/

  /**
   * @brief The == operator for Data objects
   *
   * @param [in] lhs The left hand side of equal operation
   * @param [in] rhs The right hand side of equal operation
   *
   * @return True if the two Data have the same mean
   */
  template< class T >
  inline bool operator==(const Data<T>& lhs, const Data<T>& rhs)
  {
    return lhs.mean == rhs.mean;
  }


  /**
   * @brief The != operator for Data objects
   *
   * @param [in] lhs The left hand side of equal operation
   * @param [in] rhs The right hand side of equal operation
   *
   * @return True, if not lhs == rhs
   */
  template< class T >
  inline bool operator!=(const Data<T>& lhs, const Data<T>& rhs)
  {
    return !(lhs.mean == rhs.mean);
  }


  /**
   * @brief The != operator for Distribution objects
   *
   * @param [in] lhs The left hand side of equal operation
   * @param [in] rhs The right hand side of equal operation
   *
   * @return True if the two Distribution have the same mean
   */
  template< class T >
  inline bool operator==(const Distribution<T>& lhs, const Distribution<T>& rhs)
  {
    return lhs.mean == rhs.mean;
  }


  /**
   * @brief The != operator for Distribution objects
   *
   * @param [in] lhs The left hand side of equal operation
   * @param [in] rhs The right hand side of equal operation
   *
   * @return True, if not lhs == rhs
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
     * Since the Data struct enforce the type T of being an arithmetic one,
     * we can rely on std::hash to compute its value
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
     * Since the Distribution struct enforce the type T of being an arithmetic one,
     * we can rely on std::hash to compute its value. We use only the mean value
     * to compute the hash value.
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

    /**
     * @brief Partial specialization of the has_mean trait for Data objects
     *
     * @see has_mean
     */
    template < class T >
    struct has_mean< Data<T> >
    {
      /**
       * @brief The type of the mean is equal to the mean_type of Data objects
       * @see Data
       */
      using mean_type = typename Data<T>::mean_type;

      /**
       * @brief State that the Data object implements the has_mean traits
       */
      static constexpr bool value = true;
    };


    /**
     * @brief Partial specialization of the has_mean trait for Distribution objects
     *
     * @see has_mean
     */
    template < class T >
    struct has_mean< Distribution<T> >
    {
      /**
       * @brief The type of the mean is equal to the mean_type of Distribution objects
       * @see Distribution
       */
      using mean_type = typename Distribution<T>::mean_type;

      /**
       * @brief State that the Distribution object implements the has_mean traits
       */
      static constexpr bool value = true;
    };

    /**
     * @brief Partial specialization of the has_standard_deviation trait for Distribution objects
     *
     * @see has_standard_deviation
     */
    template < class T >
    struct has_standard_deviation< Distribution<T> >
    {
      /**
       * @brief The type of the standard deviation is equal to the standard_deviation_type
       * of Distribution objects.
       * @see Distribution
       */
      using standard_deviation_type = typename Distribution<T>::standard_deviation_type;

      /**
       * @brief State that the Distribution object implements the has_standard_deviation traits
       */
      static constexpr bool value = true;
    };

  }

}

#endif // MARGOT_BASIC_INFORMATION_BLOCK_HDR
