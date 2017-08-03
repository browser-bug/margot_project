/* core/hash.hpp
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

#ifndef MARGOT_HASH_HDR
#define MARGOT_HASH_HDR


#include <cstddef>
#include <functional>


namespace margot
{

  /**
   * @brief Commmon structure to compute the hash value
   *
   * @details
   * To take advantage of hashed containers, it is required to compute
   * the hash value for the target object.
   *
   * This is the basic templated struct used by the margot framework to
   * request an hash value for a given object.
   * In particular, the framework uses the operator() to retrieve such
   * value. Therefore, if you want to add an hash function for a new
   * object you have to specilize this struct.
   *
   * If there is no implementation available for a given object, a
   * compile time error will rise.
   */
  template< class T > struct hash;




  /******************************************************************
   *  HELPER FUNCTIONS FOR COMBINE AN HASH WITH A PARTIAL VALUE
   ******************************************************************/



  /**
   * @brief Add the hash value of an object T to a partial hash
   *
   * @param [in,out] seed The partial hash value
   * @param [in] v The target object
   *
   * @details
   * This function combines a partial hash value with the hash value
   * of the target object. If there is no function that computes such
   * value, the compiler will complain about it.
   *
   * @note
   * Partially inspired by boost: the reciprocal ratio helps spread
   * entropy. (http://stackoverflow.com/questions/4948780)
   */
  template <class T>
  inline void hash_combine(std::size_t& seed, const T v)
  {
    seed ^= hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }



  /******************************************************************
   *  HELPER FUNCTIONS FOR OBJECTS WITH A STATIC SIZE
   ******************************************************************/


  /**
   * @brief Computes the hash of a static sized object
   *
   * @tparam T The type of the container (e.g. std::array)
   * @tparam object_size The static size of the object
   * @tparam index The index of the target element of the container
   *
   * @details
   * This struct is used to perform a compile-time recursion to find the
   * hash value of the whole container.
   *
   * By default this struct starts to compute the hash value from the last
   * element of the container, however this behavior might be changed
   * specifying a different value for the index parameter.
   */
  template < class T, std::size_t object_size, std::size_t index =  object_size - 1 >
  struct HashFixedSizeObject
  {
    /**
     * @brief Combine the hash value of the index-th element of the container
     *
     * @param [in,out] seed The partially computed value
     * @param [in] object The container object
     *
     * @details
     * This method compute the hash value of the index-th element and combine
     * it with a partially computed hash value.
     *
     * @note
     * The container must be accessed by std::get and must be implemented a
     * structre that compute the value of his element.
     */
    static void compute(size_t& seed, T const& object)
    {
      HashFixedSizeObject < T, object_size, index - 1 >::compute(seed, object);
      hash_combine(seed, std::get<index>(object));
    }
  };

  /**
   * @brief Computes the last hash value of a static sized object
   *
   * @tparam T The type of the container (e.g. std::array)
   * @tparam object_size The static size of the object
   *
   * @details
   * This struct is used to perform a compile-time recursion to find the
   * hash value of the whole container. In particular, it is a specialization
   * of the general struct HashFixedSizeObject and it perform the last step,
   * combining the hash value of the first element of the container.
   */
  template< class T, std::size_t object_size>
  struct HashFixedSizeObject<T, object_size, 0>
  {
    /**
     * @brief Combine the hash value of the first element of the container
     *
     * @param [in,out] seed The partially computed value
     * @param [in] object The container object
     *
     * @details
     * This method compute the hash value of the first element and combine
     * it with a partially computed hash value.
     *
     * @note
     * The container must be accessed by std::get and must be implemented a
     * structre that compute the value of his element (e.g. T::value_type)
     */
    static void compute(size_t& seed, T const& object)
    {
      hash_combine(seed, std::get<0>(object));
    }
  };


  /**
   * @brief Computes the hash value of a static-size container
   *
   * @tparam T The type of the target container
   * @tparam object_size The size of the container
   *
   * @param [in] object An l-value of the target object of type T
   *
   * @details
   * This function takes as input a fixed-size container and computes
   * its hash value. This function exploit the compile time recursion
   * of the struct HashFixedSizeObject.
   *
   * @note
   * The container must be accessed by std::get and must be implemented a
   * structre that compute the value of his element (e.g. T::value_type)
   */
  template< class T, std::size_t object_size>
  std::size_t compute_hash_fixed_size_object( T const& object )
  {
    std::size_t seed = 0;
    HashFixedSizeObject< T, object_size >::compute(seed, object);
    return seed;
  }


}

#endif // MARGOT_HASH_HDR
