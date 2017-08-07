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


      /******************************************************************
       *  TYPE DEFINITION FOR HANDLING WITH THE APPLICATION KNOWLEDGE
       ******************************************************************/

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
      using OperatingPointList = std::unordered_map< configuration_type,
            OperatingPointPtr,
            hash< configuration_type > >;


      /******************************************************************
       *  METHODS TO MANIPULATE THE OPERATING POINTS
       ******************************************************************/


      /**
       * @brief Add a new Operating Point in the application knowledge
       *
       * @param [in] new_operating_point The target Operating Point
       *
       * @return A pointer to the new Operating Point
       *
       * @details
       * If the insertion took place, i.e. the configuration of the new Operating
       * Point was not in the apllication knowledge, then this method returns a
       * pointer to the new Operating Point.
       * If the insertion never took place, i.e. there was already an Operating Point
       * with the same configuration of the new one, a null pointer is returned.
       */
      inline OperatingPointPtr add( const OperatingPoint& new_operating_point )
      {
        OperatingPointPtr new_op = OperatingPointPtr( new OperatingPoint(new_operating_point));
        const auto result_pair = knowledge.emplace(new_op->get_knobs(), new_op);
        return result_pair.second ? new_op : OperatingPointPtr{};
      }


      /**
       * @brief Add a new Operating Point in the application knowledge
       *
       * @param [in] new_operating_point A pointer to the target Operating Point
       *
       * @return A pointer to the new Operating Point
       *
       * @details
       * If the insertion took place, i.e. the configuration of the new Operating
       * Point was not in the apllication knowledge, then this method returns a
       * pointer to the new Operating Point.
       * If the insertion never took place, i.e. there was already an Operating Point
       * with the same configuration of the new one, a null pointer is returned.
       */
      inline OperatingPointPtr add( const OperatingPointPtr& new_operating_point )
      {
        const auto result_pair = knowledge.emplace(new_operating_point->get_knobs(),
                                 new_operating_point);
        return result_pair.second ? result_pair.first->second : OperatingPointPtr{};
      }


      /**
       * @brief Removes an Operating Point from the application knowledge
       *
       * @param conf [in] The configuration section of the target Operating Point
       *
       * @return A pointer to the removed Operating Point
       *
       * @details
       * The software knob section of an Operating Point is used as unique identifier
       * of the whole Operating Point. To check if two key are equal, is used the
       * == operator. Therefore, only the mean value of each software knob is
       * considered.
       * If we have removed an Operting Point, this method returns a pointer to
       * the target Operating Point. Otherwise, a nullptr is returned.
       */
      inline OperatingPointPtr remove( const configuration_type& conf )
      {
        auto op_ref = knowledge.find(conf);

        if (op_ref != knowledge.end())
        {
          const auto removed_op = op_ref->second;
          knowledge.erase(op_ref);
          return removed_op;
        }
        else
        {
          return OperatingPointPtr{};
        }
      }


      /**
       * @brief Replace the current knowledge with a new one
       *
       * @param new_list [in] The new Operating Point list.
       *
       * @details
       * This method actually swap the content of current Operating Point list
       * with the content of the new Operating Point list.
       * Therefore, the parameter new_list will contain the previous knowledge.
       */
      inline void set( OperatingPointList& new_list )
      {
        knowledge.swap(new_list);
      }


      /**
       * @brief Removes all the Operating Points from the application knwoledge
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
