/* core/operating_point.hpp
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
#ifndef MARGOT_OPERATING_POINT_HDR
#define MARGOT_OPERATING_POINT_HDR


#include <cstddef>
#include <type_traits>
#include <memory>


#include "margot/traits.hpp"
#include "margot/basic_information_block.hpp"
#include "margot/operating_point_segment.hpp"


namespace margot
{

  /**
   * @brief Definition of all the segments of an Operating Point
   *
   * @details
   * While software knobs and metrics are directly part of the definition of
   * Operating Point, input features have a more loose relation with it.
   */
  enum class DataOperatingPointSegments
  {
    SOFTWARE_KNOBS,
    METRICS
  };



  /**
   * @brief The Operating Point, building block for the application knowledge
   *
   * @tparam SoftwareKnobsSegmentType The type of the software knob segment @see OperatingPointSegment
   * @tparam MetricsSegmentType The type of the metric of interest @see MetricsSegmentType
   *
   * @details
   * This class represents an Operating Point, which relates a configuration
   * with the perfomance of the application using that configuration.
   * This class expose a unified interface, regardless of the fact the the software knobs
   * segment or the metrics segment are a Distribution or a known value.
   */
  template< class SoftwareKnobsSegmentType,
            class MetricsSegmentType >
  class OperatingPoint
  {

      // statically check if the classes have the proper traits
      static_assert(traits::is_operating_point_segment<SoftwareKnobsSegmentType>::value,
                    "Error: the software knob segment is not a valid Operating Point segment");
      static_assert(traits::is_operating_point_segment<MetricsSegmentType>::value,
                    "Error: the metric segment is not a valid Operating Point segment");

    private:

      SoftwareKnobsSegmentType software_knobs;
      MetricsSegmentType metrics;

    public:

      using metric_value_type = decltype( typename MetricsSegmentType::mean_value_type{}
                                          + typename MetricsSegmentType::standard_deviation_value_type {} );
      using software_knobs_value_type = decltype( typename SoftwareKnobsSegmentType::mean_value_type{}
                                        + typename SoftwareKnobsSegmentType::standard_deviation_value_type{} );

      /**
       * @brief Default constructor of the class
       *
       * @param knobs [in] The software knobs segment of the Operating Point
       * @param metrics [in] The metrics segment of the Operating Point
       */
      OperatingPoint(SoftwareKnobsSegmentType knobs, MetricsSegmentType metrics):
        software_knobs(knobs), metrics(metrics) {}

      OperatingPoint( const OperatingPoint& other ) = delete;
      OperatingPoint( OperatingPoint&& other ) = delete;


      /**
       * @brief This methods retrieves the configuration of the Operating Point
       *
       * @return A copy of the software knobs segment
       */
      inline SoftwareKnobsSegmentType get_knobs( void ) const
      {
        return software_knobs;
      }

      /**
       * @brief Retrives a lower bound on the value of the target metric
       *
       * @tparam metric_index The index of the target metric
       * @tparam sigma The number of times the standard deviation is considered
       *
       * @return The actual value of the metric
       *
       * @details
       * This methods computes the lower bound of a metric as it's mean minus sigma
       * times the standard deviation.
       * If the underlying type of the metric doesn't have a standard deviation, then
       * the lower bound is equal to the upper bound, which is equal to the mean.
       * Since this information is known a compile time, that part should be optimized
       * out in the final binary, according to the optimization level in the compile flags.
       */
      template< std::size_t metric_index, int sigma = 3 >
      inline metric_value_type get_metric_lower_bound( void ) const
      {
        static_assert(metric_index < MetricsSegmentType::size,
                      "Error: Index out of bound accessing the metric segment");
        return metrics.template get_mean<metric_index>() - sigma * (metrics.template get_standard_deviation<metric_index>());
      }

      /**
       * @brief Retrives a upper bound on the value of the target metric
       *
       * @tparam metric_index The index of the target metric
       * @tparam sigma The number of times the standard deviation is considered
       *
       * @return The actual value of the metric
       *
       * @details
       * This methods computes the upper bound of a metric as it's mean minus sigma
       * times the standard deviation.
       * If the underlying type of the metric doesn't have a standard deviation, then
       * the lower bound is equal to the upper bound, which is equal to the mean.
       * Since this information is known a compile time, that part might be optimized
       * out in the final binary, according to the optimization level in the compile flags.
       */
      template< std::size_t metric_index, int sigma = 3 >
      inline metric_value_type get_metric_upper_bound( void ) const
      {
        static_assert(metric_index < MetricsSegmentType::size,
                      "Error: Index out of bound accessing the metric segment");
        return metrics.template get_mean<metric_index>() + sigma * (metrics.template get_standard_deviation<metric_index>());
      }

      /**
       * @brief Retrives a lower bound on the value of the target knob
       *
       * @tparam knob_index The index of the target knob
       * @tparam sigma The number of times the standard deviation is considered
       *
       * @return The actual value of the knob
       *
       * @details
       * This methods computes the lower bound of a knob as it's mean minus sigma
       * times the standard deviation.
       * If the underlying type of the knob doesn't have a standard deviation, then
       * the lower bound is equal to the upper bound, which is equal to the mean.
       * Since this information is known a compile time, that part should be optimized
       * out in the final binary, according to the optimization level in the compile flags.
       */
      template< std::size_t knob_index, int sigma = 3 >
      inline software_knobs_value_type get_knob_lower_bound( void ) const
      {
        static_assert(knob_index < SoftwareKnobsSegmentType::size,
                      "Error: Index out of bound accessing the software knob segment");
        return software_knobs.template get_mean<knob_index>() - sigma * (software_knobs.template get_standard_deviation<knob_index>());
      }

      /**
       * @brief Retrives a upper bound on the value of the target knob
       *
       * @tparam knob_index The index of the target knob
       * @tparam sigma The number of times the standard deviation is considered
       *
       * @return The actual value of the knob
       *
       * @details
       * This methods computes the upper bound of a knob as it's mean minus sigma
       * times the standard deviation.
       * If the underlying type of the knob doesn't have a standard deviation, then
       * the lower bound is equal to the upper bound, which is equal to the mean.
       * Since this information is known a compile time, that part should be optimized
       * out in the final binary, according to the optimization level in the compile flags.
       */
      template< std::size_t knob_index, int sigma = 3 >
      inline software_knobs_value_type get_knob_upper_bound( void ) const
      {
        static_assert(knob_index < SoftwareKnobsSegmentType::size,
                      "Error: Index out of bound accessing the software knob segment");
        return software_knobs.template get_mean<knob_index>() + sigma * (software_knobs.template get_standard_deviation<knob_index>());
      }
  };

}

#endif // MARGOT_OPERATING_POINT_HDR
