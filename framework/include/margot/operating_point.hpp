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
#include <cassert>


#include "margot/traits.hpp"
#include "margot/basic_information_block.hpp"
#include "margot/operating_point_segment.hpp"
#include "margot/enums.hpp"


namespace margot
{

  /**
   * @brief The Operating Point, building block for the application knowledge
   *
   * @tparam SoftwareKnobsSegmentType The type of the software knob segment
   * @tparam MetricsSegmentType The type of the metric of interest segment
   *
   * @see OperatingPointSegment
   *
   * @details
   * This class represents an Operating Point, which relates a configuration
   * with the perfomance of the application using that configuration.
   * This class expose a unified interface, regardless of the fact the the software knobs
   * segment or the metrics segment implements the has_mean or has_standard_deviation traits.
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

      /**
       * @brief The definition of the software knobs segment
       */
      SoftwareKnobsSegmentType software_knobs;

      /**
       * @brief The definition of the metrics segment
       */
      MetricsSegmentType metrics;

    public:


      /**
       * @brief Explicitly set the configuration type
       */
      using configuration_type = SoftwareKnobsSegmentType;


      /**
       * @brief Explicitly set the configuration type
       */
      using metrics_type = MetricsSegmentType;


      /**
       * @brief The number of software knobs
       */
      static constexpr std::size_t number_of_software_knobs = SoftwareKnobsSegmentType::size;

      /**
       * @brief The number of metrics
       */
      static constexpr std::size_t number_of_metrics = MetricsSegmentType::size;


      /**
       * @brief The type of a metric field
       *
       * @details
       * If the metric segment implements the has_standard_deviation trait, then the type
       * of the field must be deducted by the type promotion of adding a mean value and a
       * standard deviation type.
       * If the metric segment doesn't implement the has_standard_deviation trait, then the
       * type of the field is the one of the mean.
       */
      using metric_value_type = decltype( typename MetricsSegmentType::mean_type{}
                                          + typename MetricsSegmentType::standard_deviation_type {} );

      /**
       * @brief The type of a software knob field
       *
       * @details
       * If the software knob segment implements the has_standard_deviation trait, then the type
       * of the field must be deducted by the type promotion of adding a mean value and a
       * standard deviation type.
       * If the software knob segment doesn't implement the has_standard_deviation trait, then the
       * type of the field is the one of the mean.
       */
      using software_knobs_value_type = decltype( typename SoftwareKnobsSegmentType::mean_type{}
                                        + typename SoftwareKnobsSegmentType::standard_deviation_type{} );

      /**
       * @brief Default constructor of the class which initialize the two segments
       *
       * @param [in] knobs The software knobs segment of the Operating Point
       * @param [in] metrics The metrics segment of the Operating Point
       */
      OperatingPoint(SoftwareKnobsSegmentType knobs, MetricsSegmentType metrics):
        software_knobs(knobs), metrics(metrics) {}


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


  /******************************************************************
   *  SPECIALIZATION OF THE TRAITS STRUCTS
   ******************************************************************/

  namespace traits
  {

    /**
     * @brief Partial specialization of the is_operating_point trait for OperatingPoint objects
     *
     * @see is_operating_point
     */
    template< class SoftwareKnobsSegmentType, class MetricsSegmentType >
    struct is_operating_point< OperatingPoint< SoftwareKnobsSegmentType, MetricsSegmentType > >
    {
      /**
       * @brief State that the OperatingPoint object implements the is_operating_point traits
       */
      static constexpr bool value = true;
    };

  }


  /******************************************************************
   *  HELPER FUNCTIONS FOR AN OPERATING POINT POINTER
   ******************************************************************/


  /**
   * @brief Helper struct, to retrieve a field of the Operating Point
   *
   * @tparam OperatingPoint The type of the target Operating Point
   * @tparam target_segment The segment of interest of the Operating Point
   * @tparam target_bound Tells if we are interested on the lower or upper bound of the field
   *
   * @see OperatingPointSegments
   * @see BoundType
   * @see OperatingPoint
   *
   * @details
   * This struct represents a unified interface to extract the value of a field of
   * the Operating Point. For example, it express the concept of "i am interested on
   * the lower bound of a metric".
   * This struct takes advantage of partial specialization to select the correct
   * method from the ones exposed by the OperatingPoint class. Since this class
   * represents the general case, you should never be able to use this struct.
   */
  template< class OperatingPoint, OperatingPointSegments target_segment, BoundType target_bound >
  struct op_utils
  {

    static_assert(traits::is_operating_point<OperatingPoint>::value,
                  "Error: the getter handles object with is_operating_point trait");

    /**
     * @brief The type of the target field
     *
     * @details
     * Since this is the general case, this type is never used, therefore it is
     * arbitrarily set to int.
     */
    using value_type = int;

    /**
     * @brief Retrive the value of the target field
     *
     * @tparam field_index The index of the field
     * @tparam sigma The number of times the standard deviation is taken into account
     *
     * @param [in] op A shared pointer to the target OperatingPoint
     *
     * @details
     * Since this struct represents the general case, you should never use this
     * method. To enforce this behavior, it always trigger an assertion to
     * terminate the process.
     */
    template< std::size_t field_index, int sigma >
    inline static value_type get( const std::shared_ptr< OperatingPoint >& op )
    {
      assert(false && "Error: unable to extract the target value from the Operating Point");
      return value_type{};
    }
  };


  /**
   * @brief Specialization of the helper struct, to retrieve the lower bound of a metric
   *
   * @tparam OperatingPoint The type of the target Operating Point
   *
   * @see op_utils
   */
  template< class OperatingPoint >
  struct op_utils< OperatingPoint, OperatingPointSegments::METRICS, BoundType::LOWER >
  {

    static_assert(traits::is_operating_point<OperatingPoint>::value,
                  "Error: the getter handles object with is_operating_point trait");

    /**
     * @brief The type of the target field, which is equal to OperatingPoint::metric_value_type
     */
    using value_type = typename OperatingPoint::metric_value_type;

    /**
     * @brief Retrive the value of the lower bound of the target metric
     *
     * @tparam field_index The index of the target metric
     * @tparam sigma The number of times the standard deviation is taken into account
     *
     * @param [in] op A shared pointer to the target OperatingPoint
     *
     * @return The value of the lower bound of the target metric, in the target OperatingPoint
     *
     * @details
     * This specialization express the concept of "get the lower bound value of the metric
     * with index field_index", in the context of the target OperatingPoint.
     * Where The lower bound is defined as the average value of the metric minus sigma
     * times its standard deviation.
     */
    template< std::size_t field_index, int sigma >
    inline static value_type get( const std::shared_ptr< OperatingPoint >& op )
    {
      return op->template get_metric_lower_bound<field_index, sigma>();
    }
  };


  /**
   * @brief Specialization of the helper struct, to retrieve the upper bound of a metric
   *
   * @tparam OperatingPoint The type of the target Operating Point
   *
   * @see op_utils
   */
  template< class OperatingPoint >
  struct op_utils< OperatingPoint, OperatingPointSegments::METRICS, BoundType::UPPER >
  {
    static_assert(traits::is_operating_point<OperatingPoint>::value,
                  "Error: the getter handles object with is_operating_point trait");

    /**
     * @brief The type of the target field, which is equal to OperatingPoint::metric_value_type
     */
    using value_type = typename OperatingPoint::metric_value_type;

    /**
     * @brief Retrive the value of the upper bound of the target metric
     *
     * @tparam field_index The index of the target metric
     * @tparam sigma The number of times the standard deviation is taken into account
     *
     * @param [in] op A shared pointer to the target OperatingPoint
     *
     * @return The value of the upper bound of the target metric, in the target OperatingPoint
     *
     * @details
     * This specialization express the concept of "get the upper bound value of the metric
     * with index field_index", in the context of the target OperatingPoint.
     * Where The upper bound is defined as the average value of the metric plus sigma
     * times its standard deviation.
     */
    template< std::size_t field_index, int sigma >
    inline static value_type get( const std::shared_ptr< OperatingPoint >& op )
    {
      return op->template get_metric_upper_bound<field_index, sigma>();
    }
  };


  /**
   * @brief Specialization of the helper struct, to retrieve the lower bound of a software knob
   *
   * @tparam OperatingPoint The type of the target Operating Point
   *
   * @see op_utils
   */
  template< class OperatingPoint >
  struct op_utils< OperatingPoint, OperatingPointSegments::SOFTWARE_KNOBS, BoundType::LOWER >
  {

    static_assert(traits::is_operating_point<OperatingPoint>::value,
                  "Error: the getter handles object with is_operating_point trait");

    /**
     * @brief The type of the target field, which is equal to OperatingPoint::software_knobs_value_type
     */
    using value_type = typename OperatingPoint::software_knobs_value_type;

    /**
     * @brief Retrive the value of the lower bound of the target software knob
     *
     * @tparam field_index The index of the target software knob
     * @tparam sigma The number of times the standard deviation is taken into account
     *
     * @param [in] op A shared pointer to the target OperatingPoint
     *
     * @return The value of the lower bound of the target software knob, in the target OperatingPoint
     *
     * @details
     * This specialization express the concept of "get the lower bound value of the software knob
     * with index field_index", in the context of the target OperatingPoint.
     * Where The lower bound is defined as the average value of the knob minus sigma
     * times its standard deviation.
     */
    template< std::size_t field_index, int sigma >
    inline static value_type get( const std::shared_ptr< OperatingPoint >& op )
    {
      return op->template get_knob_lower_bound<field_index, sigma>();
    }
  };


  /**
   * @brief Specialization of the helper struct, to retrieve the upper bound of a software knob
   *
   * @tparam OperatingPoint The type of the target Operating Point
   *
   * @see op_utils
   */
  template< class OperatingPoint >
  struct op_utils< OperatingPoint, OperatingPointSegments::SOFTWARE_KNOBS, BoundType::UPPER >
  {

    static_assert(traits::is_operating_point<OperatingPoint>::value,
                  "Error: the getter handles object with is_operating_point trait");

    /**
     * @brief The type of the target field, which is equal to OperatingPoint::software_knobs_value_type
     */
    using value_type = typename OperatingPoint::software_knobs_value_type;

    /**
     * @brief Retrive the value of the upper bound of the target software knob
     *
     * @tparam field_index The index of the target software knob
     * @tparam sigma The number of times the standard deviation is taken into account
     *
     * @param [in] op A shared pointer to the target OperatingPoint
     *
     * @return The value of the upper bound of the target software knob, in the target OperatingPoint
     *
     * @details
     * This specialization express the concept of "get the upper bound value of the software knob
     * with index field_index", in the context of the target OperatingPoint.
     * Where The upper bound is defined as the average value of the knob plus sigma
     * times its standard deviation.
     */
    template< std::size_t field_index, int sigma >
    inline static value_type get( const std::shared_ptr< OperatingPoint >& op )
    {
      return op->template get_knob_upper_bound<field_index, sigma>();
    }
  };

}

#endif // MARGOT_OPERATING_POINT_HDR
