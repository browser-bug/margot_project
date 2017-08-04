/* core/knowledge_base.hpp
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

#ifndef MARGOT_KNOWLEDGE_BASE_HDR
#define MARGOT_KNOWLEDGE_BASE_HDR

#include <unordered_map>
#include <chrono>
#include <cassert>
#include <memory>
#include <cinttypes>


#include "margot/operating_point.hpp"
#include "margot/traits.hpp"

namespace margot
{


  /**
   * @brief Reperesents the application knowledge
   *
   * @tparam number_of_software_knobs The number of software knobs
   * @tparam number_of_metrics The number of metrics
   *
   * @details
   * This class represents the application knowledge as a list of Operating
   * Points. Since it is not possible to have an order for them, this class
   * also manage views over them to sort the konwledge according to a certain
   * criteria.
   */
  template< class OperatingPoint >
  class Knowledge
  {

      // statically check the template argument
      static_assert(traits::is_operating_point<OperatingPoint>::value,
                    "Error: the knowledge base handles object with is_operating_point trait");


    public:

      /**
       * @brief Explicitly set the configuration type
       */
      using configuration_type = typename OperatingPoint::configuration_type;

      /**
       * @brief Explicitly set the configuration type
       */
      using metrics_type = typename OperatingPoint::metrics_type;

      /**
       * @brief The definition of a pointer to the actual Operating Points
       */
      using OperatingPointPtr = std::shared_ptr< OperatingPoint >;

      /**
       * @brief The container of the application knowledge
       *
       * @details
       * To facilitate operations on the knowledge base, such as adding or removing
       * Operating Point, we have chosen to use an hash table with the configuration
       * segment as the key.
       * Since we need the Operating Point information elsewhere, we are storing
       * (and copying or moving) shared pointer
       */
      using OperatingPointList = std::unordered_map< configuration_type, OperatingPointPtr >;


      /******************************************************************
       *  METHODS TO MANIPULATE THE OPERATING POINTS
       ******************************************************************/


      /**
       * @brief Add a new Operating Point in the application knowledge
       *
       * @param [in] new_operating_point The target Operating Point
       *
       * @note
       * For performance reason, this method exploit the move constructor,
       * therefore the new Operaing Point is no more valid
       */
      inline void add( OperatingPoint&& new_operating_point )
      {
        OperatingPointPtr new_op = OperatingPointPtr( new OperatingPoint(std::move(new_operating_point)));
        auto new_element = knowledge.emplace(new_op->get_knobs(), new_op);
      }


      /**
       * @brief Removes an Operating Point from the application knowledge
       *
       * @param conf [in] The configuration section of the target Operating Point
       *
       * @details
       * The software knob section of an Operating Point is used as unique identifier
       * of the whole Operating Point. To check if two key are equal, is used the
       * == operator. Therefore only the mean value of each software knob is
       * considered.
       */
      inline void remove( const configuration_type& conf )
      {
        auto op_ref = knowledge.find(conf);
        assert(op_ref != knowledge.end() && "Error: attempt to remove a non-existent Operating Point");
        knowledge.erase(op_ref);
      }


      /**
       * @brief Replace the current knowledge with a new one
       *
       * @param new_list [in] The new Operating Point list
       *
       * @details
       * It replaces the current application knowledge, however it does not destroy
       * the created view on the previous knowledge. It only updates its content.
       * It modifies the knowledge version.
       */
      inline void set( OperatingPointList new_list )
      {
        knowledge.clear();
        knowledge = std::move(new_list);
      }


      /**
       * @brief Remove all the Operating Points from the application knwoledge
       *
       * @details
       * This method erase all the Operating Points from the knowledge attributes
       * and from all the views. It does not deletes the views.
       * It modifies the knowledge version.
       */
      inline void clear( void )
      {
        knowledge.clear();
      }


      /******************************************************************
       *  UTILITY METHODS FOR THE APPLICATION KNOWLEDGE
       ******************************************************************/


      /**
       * @brief Retrieves an iterator to the start of the application knowledge
       *
       * @return A constant iterator to the begin of the knowledge
       */
      inline typename OperatingPointList::const_iterator begin( void ) const
      {
        return knowledge.begin();
      }


      /**
       * @brief Retrieves an iterator to the end of the application knowledge
       *
       * @return A constant iterator to the end of the knowledge
       */
      inline typename OperatingPointList::const_iterator end( void ) const
      {
        return knowledge.end();
      }


      /**
       * @brief Test whether the application knowledge is empty
       *
       * @return True, if the are no Operating Points in the knowledge
       */
      inline bool empty( void ) const
      {
        return knowledge.empty();
      }


      /**
       * @brief Retrieve the size of the application knowledge
       *
       * @return The number of Operating Points in the application knowledge
       */
      inline std::size_t size( void ) const
      {
        return knowledge.size();
      }


    private:

      /**
       * @brief The application knowledge
       */
      OperatingPointList knowledge;

  };



}


#endif // MARGOT_OPERATING_POINT_CONTAINERS_HDR
