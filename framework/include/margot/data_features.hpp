/* core/data_features.hpp
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

#ifndef MARGOT_DATA_FEATURES_HPP
#define MARGOT_DATA_FEATURES_HPP

#include <tuple>
#include <array>
#include <functional>

#include "margot/enums.hpp"

namespace margot
{




  /******************************************************************
   *  HELPER STRUCT TO COMPARE THE DATA FEATURE VALUES
   ******************************************************************/


  /**
   * @brief Functor that defines the "don't care" comparison function
   *
   * @tparam T The type of the "compared" objects
   *
   * @details
   * This functor is used to when the user is not interested on specifying a
   * constraint on a field of the data feature
   */
  template< class T >
  struct dont_care
  {

    /**
     * @brief Override of the call operator, to compare two objects
     *
     * @return True, as constexpression
     */
    constexpr bool operator()(const T&, const T&) const
    {
      return true;
    }

  };


  /**
   * @brief Proxy struct to select the correct comparison function
   *
   * @tparam T The type of the element stored in the data feature
   * @tparam comparison The enumerator of the target comparison function
   *
   * @details
   * This struct is used to define the correct type of the coparison function,
   * since we have the "don't care" comparison function, beside the classical
   * four std comparison functor.
   * This struct take advantage of partial specialization to select, at compile
   * time, the correct comparison function.
   */
  template< class T, FeatureComparison comparison >
  struct cf_proxy;


  /**
   * @brief Proxy struct to select the correct comparison function
   *
   * @tparam T The type of the element stored in the data feature
   *
   * @details
   * Partial specialization for the less or equal comparison
   */
  template< class T >
  struct cf_proxy< T, FeatureComparison::LESS_OR_EQUAL >
  {

    /**
     * @brief Explicit re-definition of the std::less_equal functor
     */
    using compare = std::less_equal<T>;

  };


  /**
   * @brief Proxy struct to select the correct comparison function
   *
   * @tparam T The type of the element stored in the data feature
   *
   * @details
   * Partial specialization for the greater comparison
   */
  template< class T >
  struct cf_proxy< T, FeatureComparison::GREATER_OR_EQUAL >
  {

    /**
     * @brief Explicit re-definition of the std::greater_equal functor
     */
    using compare = std::greater_equal<T>;

  };


  /**
   * @brief Proxy struct to select the correct comparison function
   *
   * @tparam T The type of the element stored in the data feature
   *
   * @details
   * Partial specialization for the "don't care" comparison function
   */
  template< class T >
  struct cf_proxy< T, FeatureComparison::DONT_CARE >
  {

    /**
     * @brief Explicit re-definition of the don't care functor
     */
    using compare = dont_care<T>;

  };




  /******************************************************************
   *  STRUCT THAT TEST WHETHER A DATA FEATURE IS ADMISSIBLE
   ******************************************************************/


  /**
   * @brief This functor is used to check if a data feature is valid wrt another one
   *
   * @tparam DataFeature The type of the data feature, it is assumed to be a std::array
   * @tparam cf The comparison type for the target field of the data feature, suppose i-th field
   * @tparam cfs The remainder of the comparison functions
   *
   * @details
   * This functor test the validity of the i-th field of a data feature. The order of
   * the cfs must reflect the order of the field of the data feature.
   * This class leverage compile-time recursion to generate the actual function that
   * perform the test, according to the size of the data feature.
   */
  template< class DataFeature, FeatureComparison cf, FeatureComparison... cfs >
  struct data_features_admissible
  {


    static_assert( sizeof...(cfs) + 1 <= DataFeature{}.size(), "Error: mismatch number of comparison function to validate a data feature");


    /**
     * @brief Explicit definition of the type of the values stored in the data feature
     */
    using value_type = typename DataFeature::value_type;


    /**
     * @brief Explicit definition of the comparison functor
     */
    using comparison_function = typename cf_proxy<value_type,cf>::compare;


    /**
     * @brief Call operator, which test whether f1 is admissible with respect to f2
     *
     * @param f1 The evaluated data feature
     * @param f2 The target data feature
     *
     * @return true, if up to the i-th field of f1 is admissible wrt f2
     */
    inline bool operator()(const DataFeature& f1, const DataFeature& f2) const
    {
      return comparison_function()(f1[f1.size() - (sizeof...(cfs) + 1)],f2[f1.size() - (sizeof...(cfs) + 1)])
        && data_features_admissible<DataFeature, cfs...>()(f1,f2);
    }

  };


  /**
   * @brief This functor is used to check if a data feature is valid wrt another one
   *
   * @tparam DataFeature The type of the data feature, it is assumed to be a std::array
   * @tparam cf The comparison type for the target field of the data feature, suppose i-th field
   * @tparam cfs The remainder of the comparison functions
   *
   * @details
   * This is a partial specialization of the previous struct, used to perform the last
   * step of the recursive compilation. In particular, it evaluates the last field of
   * the data feature.
   */
  template< class DataFeature, FeatureComparison cf >
  struct data_features_admissible<DataFeature, cf>
  {


    /**
     * @brief Explicit definition of the type of the values stored in the data feature
     */
    using value_type = typename DataFeature::value_type;


    /**
     * @brief Explicit definition of the comparison functor
     */
    using comparison_function = typename cf_proxy<value_type,cf>::compare;


    /**
     * @brief Call operator, which test whether f1 is admissible with respect to f2
     *
     * @param f1 The evaluated data feature
     * @param f2 The target data feature
     *
     * @return true, if the last field of f1 is admissible wrt f2
     */
    inline bool operator()(const DataFeature& f1, const DataFeature& f2) const
    {
      return comparison_function()(f1[f1.size() - 1],f2[f1.size() - 1]);
    }

  };




  /******************************************************************
   *  STRUCT THAT COMPUTES THE n-DIMENSIONAL DISTANCE
   ******************************************************************/


   /**
    * @brief This struct is used to compute the distance between n-dimensional data features
    *
    * @tparam DataFeature The type of the data feature, it is assumed to be a std::array
    * @tparam index The index of the target field of the data feature
    *
    * @details
    * This struct leverage compile time recursion to compute the distance between two
    * n-dimensional data features.
    */
  template< class DataFeature, std::size_t index = DataFeature{}.size() - 1  >
  struct data_features_distance
  {


    /**
     * @brief Explicit definition of the type of the values stored in the data feature
     */
    using value_type = typename DataFeature::value_type;


    /**
     * @brief Call operator, which computes the difference on the index-th field
     *
     * @param origin The reference data feature
     * @param target The target data feature
     *
     * @return the partial distance, computed up to the index-th field of data feature
     */
    inline value_type operator()(const DataFeature& origin, const DataFeature& target) const
    {
      const auto distance = std::get<index>(origin) - std::get<index>(target);
      return distance * distance + data_features_distance<DataFeature,index-1>()(origin,target);
    }

  };


  /**
   * @brief This struct is used to compute the distance between n-dimensional data features
   *
   * @tparam DataFeature The type of the data feature, it is assumed to be a std::array
   *
   * @details
   * This struct leverage compile time recursion to compute the distance between two
   * n-dimensional data features. In particular, this struct perform the last step of
   * computing the distance.
   */
  template< class DataFeature  >
  struct data_features_distance<DataFeature, 0>
  {


    /**
     * @brief Explicit definition of the type of the values stored in the data feature
     */
    using value_type = typename DataFeature::value_type;


    /**
     * @brief Call operator, which computes the difference on the first field
     *
     * @param origin The reference data feature
     * @param target The target data feature
     *
     * @return the initial distance, computed using the first field of the data feature
     */
    inline value_type operator()(const DataFeature& origin, const DataFeature& target) const
    {
      const auto distance = std::get<0>(origin) - std::get<0>(target);
      return distance * distance;
    }

  };




  /******************************************************************
   *  STRUCT THAT NORMALIZES THE VALUE OF DATA FEATURES
   ******************************************************************/


  /**
   * @brief This struct normalize the values of the given data features
   *
   * @tparam DataFeature The type of the data feature, it is assumed to be a std::array
   * @tparam index The index of the target field of the data feature
   *
   * @details
   * If the fields of data features have different magnitude, a plain euclidean distance would
   * be biased towards changes on the most dominant field. This struct will normalize the values
   * of data features between 0 and 1, to compute a more fair distance.
   * This struct leverage compile time recursion to perform the normalization in a n-dimensional
   * data feature.
   */
  template< class DataFeature, std::size_t index = DataFeature{}.size() - 1  >
  struct normalize_datafeatures
  {


    /**
     * @brief Call operator, which normalize the index-th field of the data feature
     *
     * @param origin [out] The reference data feature, e.g. the one of the actual input
     * @param f1 [out] The first data feature to compare, e.g. the proposed closer one
     * @param f2 [out] The second data feature to compare, e.g. the target one
     */
    inline void operator()(DataFeature& origin, DataFeature& f1, DataFeature& f2) const
    {
      // find the range of the spawn of the points
      const auto coord_min = std::min(std::min(std::get<index>(origin), std::get<index>(f1)), std::get<index>(f2));
      const auto coord_max = std::max(std::max(std::get<index>(origin), std::get<index>(f1)), std::get<index>(f2));
      const auto coord_width = coord_max - coord_min;

      // normalize the coordinates of the points
      std::get<index>(origin) = (std::get<index>(origin) - coord_min) / coord_width;
      std::get<index>(f1) = (std::get<index>(f1) - coord_min) / coord_width;
      std::get<index>(f2) = (std::get<index>(f2) - coord_min) / coord_width;

      // go to the lower axis
      normalize_datafeatures<DataFeature, index -1>()(origin, f1, f2);
    }

  };


  /**
   * @brief This struct normalize the values of the given data features
   *
   * @tparam DataFeature The type of the data feature, it is assumed to be a std::array
   *
   * @details
   * If the fields of data features have different magnitude, a plain euclidean distance would
   * be biased towards changes on the most dominant field. This struct will normalize the values
   * of data features between 0 and 1, to compute a more fair distance.
   * This struct leverage compile time recursion to perform the normalization in a n-dimensional
   * data feature. In particular, this struct perform the normalization of the first field of the
   * data feature.
   */
  template< class DataFeature  >
  struct normalize_datafeatures< DataFeature, 0 >
  {


    /**
     * @brief Call operator, which normalize the index-th field of the data feature
     *
     * @param origin [out] The reference data feature, e.g. the one of the actual input
     * @param f1 [out] The first data feature to compare, e.g. the proposed closer one
     * @param f2 [out] The second data feature to compare, e.g. the target one
     */
    inline void operator()(DataFeature& origin, DataFeature& f1, DataFeature& f2) const
    {
      // find the range of the spawn of the points
      const auto coord_min = std::min(std::min(std::get<0>(origin), std::get<0>(f1)), std::get<0>(f2));
      const auto coord_max = std::max(std::max(std::get<0>(origin), std::get<0>(f1)), std::get<0>(f2));
      const auto coord_width = coord_max - coord_min;

      // normalize the coordinates of the points
      std::get<0>(origin) = (std::get<0>(origin) - coord_min) / coord_width;
      std::get<0>(f1) = (std::get<0>(f1) - coord_min) / coord_width;
      std::get<0>(f2) = (std::get<0>(f2) - coord_min) / coord_width;
    }

  };




  /******************************************************************
   *  THE ACTUAL STRUCT WHICH SELECT THE CLOSER DATA FEATURE
   ******************************************************************/


  /**
   * @brief This struct selects an iterator to Asrtm, which is closer to the given data features
   *
   * @tparam DataFeature The type of the data feature, it is assumed to be a std::array
   * @tparam container_iterator The type of the DA AS-RTM container iterator
   * @tparam distance_type The way to compute the distance between data features
   * @tparam cfs The list of data features's comparison functor, which define their validity
   *
   * @details
   * This struct is in charge of selecting the closer data feature to the one of the input. This class
   * leverage partial specialization to select the correct algorithm to select the closer one.
   */
  template< class DataFeature, class container_iterator, FeatureDistanceType distance_type, FeatureComparison... cfs >
  struct data_feature_selector;


  /**
   * @brief This struct selects an iterator to Asrtm, which is closer to the given data features
   *
   * @tparam DataFeature The type of the data feature, it is assumed to be a std::array
   * @tparam container_iterator The type of the DA AS-RTM container iterator
   * @tparam cfs The list of data features's comparison functor, which define their validity
   *
   * @details
   * This struct is in charge of selecting the closer data feature to the one of the input. This class
   * leverage partial specialization to select the correct algorithm to select the closer one.
   * In particular this struct computes the distance as a pure euclidean distance.
   */
  template< class DataFeature, class container_iterator, FeatureComparison... cfs >
  struct data_feature_selector<DataFeature, container_iterator, FeatureDistanceType::EUCLIDEAN, cfs...>
  {


    /**
     * @brief Select the closer data feature to the target one
     *
     * @param origin The value of the data feature of the input
     * @param best An iterator to closest element of the DA AS-RTM, from the one of the input
     * @param target An iterator to the evaluated element of the DA AS-RTM
     *
     * @return The iterator of the closest data feature (i.e. best or target), wrt origin
     *
     * @details
     * The implementation of the method assumes that the element of the DA AS-RTM is a pair, with
     * the data feature as first element.
     */
    container_iterator operator()(const DataFeature& origin, const container_iterator& best, const container_iterator& target) const
    {
      // instantiate the object to compare the data features
      const data_features_admissible<DataFeature, cfs...> is_valid = data_features_admissible<DataFeature, cfs...>();
      const data_features_distance<DataFeature> get_distance = data_features_distance<DataFeature>();

      // check the validity
      const bool is_best_valid = is_valid(best->first, origin);
      const bool is_target_valid = is_valid(target->first, origin);

      // compare them wrt their validity
      if (is_best_valid && !is_target_valid)
      {
        return best;
      }
      if (!is_best_valid && is_target_valid)
      {
        return target;
      }

      // otherwise, we should compare them with their distance
      const auto distance_best = get_distance(origin, best->first);
      const auto distance_target = get_distance(origin, target->first);

      // in case of a tie, favor the best found
      return distance_target < distance_best ? target : best;
    }

  };


  /**
   * @brief This struct selects an iterator to Asrtm, which is closer to the given data features
   *
   * @tparam DataFeature The type of the data feature, it is assumed to be a std::array
   * @tparam container_iterator The type of the DA AS-RTM container iterator
   * @tparam cfs The list of data features's comparison functor, which define their validity
   *
   * @details
   * This struct is in charge of selecting the closer data feature to the one of the input. This class
   * leverage partial specialization to select the correct algorithm to select the closer one.
   * In particular this struct computes the distance as a normalized euclidean distance.
   */
  template< class DataFeature, class container_iterator, FeatureComparison... cfs >
  struct data_feature_selector<DataFeature, container_iterator, FeatureDistanceType::NORMALIZED, cfs...>
  {


    /**
     * @brief Select the closer data feature to the target one
     *
     * @param origin The value of the data feature of the input
     * @param best An iterator to closest element of the DA AS-RTM, from the one of the input
     * @param target An iterator to the evaluated element of the DA AS-RTM
     *
     * @return The iterator of the closest data feature (i.e. best or target), wrt origin
     *
     * @details
     * The implementation of the method assumes that the element of the DA AS-RTM is a pair, with
     * the data feature as first element.
     */
    container_iterator operator()(const DataFeature& origin, const container_iterator& best, const container_iterator& target) const
    {
      // instantiate the object to compare the data features
      const data_features_admissible<DataFeature, cfs...> is_valid = data_features_admissible<DataFeature, cfs...>();
      const data_features_distance<DataFeature> get_distance = data_features_distance<DataFeature>();
      const normalize_datafeatures<DataFeature> normalize = normalize_datafeatures<DataFeature>();

      // check the validity
      const bool is_best_valid = is_valid(best->it, origin);
      const bool is_target_valid = is_valid(target->it, origin);

      // compare them wrt their validity
      if (is_best_valid && !is_target_valid)
      {
        return best;
      }
      if (!is_best_valid && is_target_valid)
      {
        return target;
      }

      // otherwise normalize them
      auto origin_normalized = origin;
      auto best_normalized = best->first;
      auto target_normalized = target->first;
      normalize(origin_normalized, best_normalized, target_normalized);

      // otherwise, we should compare them with their distance
      const auto distance_best = get_distance(origin_normalized, best_normalized);
      const auto distance_target = get_distance(origin_normalized, target_normalized);

      // in case of a tie, favor the best found
      return distance_target < distance_best ? target : best;
    }

  };

}

#endif // MARGOT_DATA_FEATURES_HPP
