/* core/debug.hpp
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


#ifndef MARGOT_DEBUG_HDR
#define MARGOT_DEBUG_HDR

#include <vector>
#include <string>
#include <iostream>
#include <cstddef>

#include "margot/operating_point.hpp"
#include "margot/knowledge_base.hpp"


namespace margot
{

  /**
   * @brief Print the main margot header
   *
   * @details
   * This method is used to print to the standard output the main header of
   * margot, to debug purpose
   */
  inline void print_header( void )
  {
    std::cout << std::endl << std::endl << std::endl;
    std::cout << "#####################################################################" << std::endl;
    std::cout << "#                                                                   #" << std::endl;
    std::cout << "#                     ___________________________________           #" << std::endl;
    std::cout << "#          _______ ______    |__  __ \\_  ____/_  __ \\_  /_          #" << std::endl;
    std::cout << "#          __  __ `__ \\_  /| |_  /_/ /  / __ _  / / /  __/          #" << std::endl;
    std::cout << "#          _  / / / / /  ___ |  _, _// /_/ / / /_/ // /_            #" << std::endl;
    std::cout << "#          /_/ /_/ /_//_/  |_/_/ |_| \\____/  \\____/ \\__/            #" << std::endl;
    std::cout << "#                                                                   #" << std::endl;
    std::cout << "#                 Dynamic Autotuner Framework v 2.0                 #" << std::endl;
    std::cout << "#####################################################################" << std::endl;
    std::cout << "#" << std::endl;
    std::cout << "#" << std::endl;
  }


  /**
   * @brief Print the trailer of the dump
   */
  inline void print_trailer( void )
  {
    std::cout << "#" << std::endl;
    std::cout << "#" << std::endl;
    std::cout << "#####################################################################" << std::endl;
    std::cout << "#                         That's all folks!                         #" << std::endl;
    std::cout << "#####################################################################" << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;
  }

  template< class OperatingPointSegment, std::size_t number_of_elements, std::size_t current_index = number_of_elements - 1 >
  struct segment_to_string
  {
    static void fill_configuration( const OperatingPointSegment& segment,  std::vector< std::string >& description )
    {
      // convert to string the current field
      const auto mean_value = segment.template get_mean<current_index>();
      const auto std_dev = segment.template get_standard_deviation<current_index>();
      const std::string mean_value_str = std::to_string(mean_value);
      description[current_index] = std_dev != 0 ? mean_value_str + " +- " + std::to_string(std_dev) : mean_value_str;

      // recursively compute the other fields
      segment_to_string<OperatingPointSegment,number_of_elements,current_index -1>::fill_configuration(segment, description);
    }
  };

  template< class OperatingPointSegment, std::size_t number_of_elements >
  struct segment_to_string<OperatingPointSegment,number_of_elements,0>
  {
    static void fill_configuration( const OperatingPointSegment& segment,  std::vector< std::string >& description )
    {
      // convert to string the current field
      const auto mean_value = segment.template get_mean<0>();
      const auto std_dev = segment.template get_standard_deviation<0>();
      const std::string mean_value_str = std::to_string(mean_value);
      description[0] = std_dev != 0 ? mean_value_str + " +- " + std::to_string(std_dev) : mean_value_str;
    }
  };

  inline std::string pad_string( std::string output, std::size_t max_size )
  {
    const std::size_t string_size = output.length();
    if ( string_size < max_size )
    {
      const std::size_t padding_lenght = max_size - string_size;
      const std::size_t left_padding = padding_lenght / 2;
      const std::size_t right_padding = padding_lenght - left_padding;
      const std::string left_pad(left_padding, ' ');
      const std::string right_pad(right_padding, ' ');
      return left_pad + output + right_pad;
    }
    return output;
  }


  /**
   * @brief Pretty print the whole Operating Point
   */
  template< class OperatingPoint >
  inline void print_whole_op( const typename Knowledge<OperatingPoint>::OperatingPointPtr& op, const std::string prefix )
  {
    // size of the dump for the Operating Point
    const std::size_t op_width = 30;
    const std::string line_seperator(op_width, '-');

    // get the geometry of the Operating Points
    using software_knob_segment_t = typename OperatingPoint::configuration_type;
    using metric_segment_t = typename OperatingPoint::metrics_type;

    // get the segment of the target Operating Point
    const auto software_knobs = op->get_knobs();
    const auto metrics = op->get_metrics();

    // print the header of the software knob
    std::cout << prefix << " +" << line_seperator << "+" << std::endl;
    std::cout << prefix << " |" << pad_string("Software Knobs",op_width) << "|" << std::endl;
    std::cout << prefix << " +" << line_seperator << "+" << std::endl;


    // get the to_string representation of the Operating Point
    std::vector< std::string > knob_values(OperatingPoint::number_of_software_knobs);
    segment_to_string<software_knob_segment_t,OperatingPoint::number_of_software_knobs>::fill_configuration(software_knobs, knob_values);

    // print the informations
    for( const auto& knob_value : knob_values )
    {
      std::cout << prefix << " |" << pad_string(knob_value ,op_width) << "|" << std::endl;
    }

    // print the header of the metrics
    std::cout << prefix << " +" << line_seperator << "+" << std::endl;
    std::cout << prefix << " |" << pad_string("Metrics",op_width) << "|" << std::endl;
    std::cout << prefix << " +" << line_seperator << "+" << std::endl;


    // get the to_string representation of the Operating Point
    std::vector< std::string > metric_values(OperatingPoint::number_of_metrics);
    segment_to_string<metric_segment_t,OperatingPoint::number_of_metrics>::fill_configuration(metrics, metric_values);

    // print the informations
    for( const auto& metric_value : metric_values )
    {
      std::cout << prefix << " |" << pad_string(metric_value,op_width) << "|" << std::endl;
    }


    // print the trailer of the Operating Point
    std::cout << prefix << " +" << line_seperator << "+" << std::endl;
  }


  /**
   * @brief Pretty print the whole Operating Point
   */
  template< class OperatingPoint, typename value_type >
  inline void print_conf_with_value( const typename Knowledge<OperatingPoint>::OperatingPointPtr& op, const value_type value, const std::string& prefix, const std::string& label )
  {
    // size of the dump for the Operating Point
    const std::size_t op_width = 30;
    const std::string line_seperator(op_width, '-');

    // get the geometry of the Operating Points
    using software_knob_segment_t = typename OperatingPoint::configuration_type;

    // get the segment of the target Operating Point
    const auto software_knobs = op->get_knobs();

    // print the header of the software knob
    std::cout << prefix << " +" << line_seperator << "+" << std::endl;
    std::cout << prefix << " |" << pad_string("Software Knobs",op_width) << "|" << std::endl;
    std::cout << prefix << " +" << line_seperator << "+" << std::endl;


    // get the to_string representation of the Operating Point
    std::vector< std::string > knob_values(OperatingPoint::number_of_software_knobs);
    segment_to_string<software_knob_segment_t,OperatingPoint::number_of_software_knobs>::fill_configuration(software_knobs, knob_values);

    // print the informations
    for( const auto& knob_value : knob_values )
    {
      std::cout << prefix << " |" << pad_string(knob_value ,op_width) << "|" << std::endl;
    }

    // print the header of the metrics
    std::cout << prefix << " +" << line_seperator << "+" << std::endl;
    std::cout << prefix << " |" << pad_string(label,op_width) << "|" << std::endl;
    std::cout << prefix << " +" << line_seperator << "+" << std::endl;
    std::cout << prefix << " |" << pad_string(std::to_string(value),op_width) << "|" << std::endl;
    std::cout << prefix << " +" << line_seperator << "+" << std::endl;
  }

}

#endif // MARGOT_DEBUG_HDR
