/* core/knowledge_adaptor.hpp
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

#ifndef MARGOT_KNOWLEDGE_ADAPTOR_HDR
#define MARGOT_KNOWLEDGE_ADAPTOR_HDR

#include <memory>
#include <cinttypes>
#include <array>
#include <string>
#include <iostream>

#include "margot/operating_point.hpp"
#include "margot/traits.hpp"
#include "margot/enums.hpp"
#include "margot/field_adaptor.hpp"
#include "margot/knowledge_base.hpp"

namespace margot
{

  /**
   * @brief This class manages the FieldAdaptor objects
   *
   * @see FieldAdaptor
   *
   * @tparam OperatingPoint The type of the target Operating Point
   * @tparam coefficient_type The type of error coefficient
   *
   * @details
   * This class aims at relating each field of the Operating Point with runtime
   * information coming from the monitors.
   * In particular, this class collects all the FieldAdaptor objects that are
   * created to adapt the Knowledge.
   */
  template< class OperatingPoint, typename coefficient_type = float >
  class KnowledgeAdaptor
  {


      // statically check the template argument
      static_assert(traits::is_operating_point<OperatingPoint>::value,
                    "Error: the knowledge adaptor handles object with is_operating_point trait");


    public:


      /**
       * @brief Explicit definition to an Operating Point pointer
       */
      using OperatingPointPtr = typename Knowledge<OperatingPoint>::OperatingPointPtr;


      /**
       * @brief Explicit definition to a FieldAdaptor pointer
       */
      using FieldAdaptorPtr = std::shared_ptr< FieldAdaptor< OperatingPoint, coefficient_type> >;


      /**
       * @brief Definition of the FieldAdaptor container
       *
       * @details
       * Since we are interested only on the average value of a monitor, it is possible to have, at most,
       * only one FieldAdaptor for each field of the Operating Point.
       */
      using Container = std::array < FieldAdaptorPtr, OperatingPoint::number_of_software_knobs + OperatingPoint::number_of_metrics >;


      /**
       * @brief Default constructor, initialize all the adaptors to a null pointer
       */
      KnowledgeAdaptor( void )
      {
        adaptors.fill(nullptr);
      }


      /**
       * @brief Allocate and emplace a FieldAdaptor
       *
       * @tparam target_segment The segment of interest of the Operating Point
       * @tparam target_field_index The index of the target field
       * @tparam inertia The size of the buffer used by the FieldAdaptor
       * @tparam T The type of elements stored in the monitor
       * @tparam statistical_t The minimum type used by the monitor to compute the average
       *
       * @param [in] monitor The monitor related to the target Operating Point field
       *
       * @details
       * This class create a new FieldAdaptor object and relate it to the target field of
       * the Operating Point. It overwrites a previous FieldAdaptor.
       */
      template< OperatingPointSegments target_segment,
                std::size_t target_field_index,
                std::size_t inertia,
                class T,
                typename statistical_t >
      inline void emplace( const Monitor<T, statistical_t>& monitor )
      {
        adaptors[op_field_enumerator< OperatingPoint, target_segment, target_field_index>::get()].reset(
          new OneSigmaAdaptor<OperatingPoint, target_segment, target_field_index, inertia, coefficient_type>(monitor)
        );
      }


      /**
       * @brief Retrieve the target FieldAdaptor
       *
       * @tparam target_segment The target segment of the Operating Point
       * @tparam target_field_index The index of the target field whithin the the target segment
       *
       * @return A shared pointer to the related FieldAdaptor, if any.
       *
       * @note
       * If there is monitor related to the target field, this method returns a null pointer
       */
      template< OperatingPointSegments target_segment, std::size_t target_field_index >
      inline FieldAdaptorPtr get_field_adaptor( void ) const
      {
        return adaptors[op_field_enumerator< OperatingPoint, target_segment, target_field_index>::get()];
      }


      /**
       * @brief Update the runtime information according to the current Operating Point
       *
       * @param [in] current_op Shared pointer to the Operating Point used by the application
       *
       * @detials
       * This method updates all the registered field adaptor taking into account runtime information.
       * In order to compute the correct value, we must be sure that the application is using the
       * Operating Point specified in the parameter
       */
      void evaluate_error( const OperatingPointPtr& current_op )
      {
        // loop over all the adaptors
        for ( auto& adaptor : adaptors)
        {
          // if it is associated any runtime information
          if (adaptor)
          {
            // update the coefficient error
            adaptor->evaluate_error(current_op);
          }
        }
      }


      /**
       * @brief Clear all the observation from the monitors
       *
       * @details
       * This method is used to reset the state of all the field adaptor available,
       * i.e. fill their buffer with the value 1. Typically it is used to clone an
       * Asrtm for a new feature cluster
       */
      inline void reset( void )
      {
        for ( auto& adaptor : adaptors)
        {
          // if it is associated any runtime information
          if (adaptor)
          {
            // reset the observations
            adaptor->clear_observations();
          }
        }
      }


      /**
       * @brief Removes all the references to runtime information.
       *
       * @details
       * This methods resets all the adaptors previously used. The effect of this method is to
       * delete all the references to the Monitor and the circular buffer used to compute the
       * error coefficients as well.
       */
      inline void clear( void )
      {
        adaptors.fill(nullptr);
      }


      /**
       * @brief Print the status of the runtime information provider
       */
      void dump( const std::string& prefix ) const;


    private:


      /**
       * @brief The actual container of the adaptors
       */
      Container adaptors;

  };




  template< class OperatingPoint, typename coefficient_type >
  void KnowledgeAdaptor<OperatingPoint,coefficient_type>::dump( const std::string& prefix ) const
  {
    // print the header
    std::cout << prefix << std::endl;
    std::cout << prefix << " List of runtime information providers for software knobs:" << std::endl;
    std::cout << prefix << std::endl;

    // print all the providers
    std::size_t counter = 0;
    for( const auto& provider : adaptors )
    {
      if (counter == OperatingPoint::number_of_software_knobs)
      {
        std::cout << prefix << std::endl;
        std::cout << prefix << " List of runtime information providers for metrics:" << std::endl;
        std::cout << prefix << std::endl;
      }
      std::cout << prefix << "\tField index " << std::to_string(counter) << " -> ";
      if (provider)
      {
        std::cout << provider->get_status();
      }
      else
      {
        std::cout << "N/A";
      }
      std::cout << std::endl;
      ++counter;
    }

    // add some spaces
    std::cout << prefix << std::endl;

  }

}

#endif // MARGOT_KNOWLEDGE_ADAPTOR_HDR
