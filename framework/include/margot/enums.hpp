/* core/enums.hpp
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

#ifndef MARGOT_ENUMS_HDR
#define MARGOT_ENUMS_HDR

namespace margot
{

  /**
   * @brief Used to select the Operating Point segment of interest
   *
   * @details
   * This enum states all the available segments of an
   * OperatingPoint.
   */
  enum class OperatingPointSegments
  {
    SOFTWARE_KNOBS,
    METRICS
  };


  /**
   * @brief Used to select the statistical property of interest
   *
   * @details
   * This enum states all the available statistical properties
   * exposed by the StatisticalProvider.
   */
  enum class DataFunctions
  {
    AVERAGE,
    STANDARD_DEVATION,
    MAXIMUM,
    MINIMUM
  };


  /**
   * @brief Used to select the comparison function of interest
   *
   * @details
   * This enum states all the comparison functions supported by
   * the mARGOt framework.
   */
  enum class ComparisonFunctions
  {
    GREATER,
    GREATER_OR_EQUAL,
    LESS,
    LESS_OR_EQUAL
  };


  /**
   * @brief Used to select the bound of interest
   *
   * @details
   * This enum is internally used to select if we are interested on the
   * upper bound of a field of the OperatingPoint, or on its lower bound.
   */
  enum class BoundType
  {
    LOWER,
    UPPER
  };


  /**
   * @brief Used to select the direction of the objective function
   *
   * @details
   * This enum is used to select if we want to maximize or minimize
   * the objective function, i.e. the rank.
   */
  enum class RankObjective
  {
    MAXIMIZE,
    MINIMIZE
  };


  /**
   * @brief Used to indicate how to compose the objective function
   *
   * @details
   * This enum is used to select how to compose the fields of the
   * objective function.
   *
   * @see Rank
   */
  enum class FieldComposer
  {
    GEOMETRIC,
    LINEAR,
    SIMPLE
  };


  /**
   * @brief Used to indicate the unit of measure for times interval
   *
   * @details
   * This enum is used in the application monitor, to select the granularity
   * of the measurements
   */
  enum class TimeUnit
  {
    NANOSECONDS,
    MICROSECONDS,
    MILLISECONDS,
    SECONDS
  };


  /**
   * @brief Used to indicate the comparison function for the data features
   *
   * @details
   * This enum replace the classical comparison function with the "don't care"
   * enumerator, which indicates that the user has no constraints for the
   * given field of the data feature.
   * Moreover, in case that the 
   */
  enum class FeatureComparison
  {
    LESS_OR_EQUAL,
    GREATER_OR_EQUAL,
    DONT_CARE
  };


  /**
   * @brief Used to indicate the type of distance between data features
   */
  enum class FeatureDistanceType
  {
    /**
     * @details
     * This enumerator defines the "classic" euclidean distance in n-dimensional space
     */
    EUCLIDEAN,

    /**
     * @details
     * This enumerator is similar to previous one, but all the distances are normalized.
     * It is usefull when the fields of the data features have different magnitude.
     */
    NORMALIZED
  };

}

#endif // MARGOT_ENUMS_HDR
