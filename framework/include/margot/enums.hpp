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
  enum class ComparisonFunction
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


  enum class RankObjective
  {
    MAXIMIZE,
    MINIMIZE
  };


  enum class FieldComposer
  {
    GEOMETRIC,
    LINEAR,
    SIMPLE
  };


}

#endif // MARGOT_ENUMS_HDR
