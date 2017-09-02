/* core/view
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


#ifndef MARGOT_VIEW_HDR
#define MARGOT_VIEW_HDR


#include <map>
#include <memory>
#include <vector>
#include <functional>
#include <cstddef>
#include <cassert>
#include <algorithm>


#include "margot/operating_point.hpp"
#include "margot/evaluator.hpp"
#include "margot/knowledge_base.hpp"


namespace margot
{

  /**
   * @brief An organized view of the knowledge base
   *
   * @tparam OperatingPoint The type of the target Operating Point
   * @tparam composer The way of combining the fields that evaluates an Operating Point
   * @tparam ...Fields The list of fields of interest to evaluate an Operating Point
   *
   * @see Evaluator
   *
   * @details
   * This class organizes the application knowledge according to the number computed
   * by the Evaluator of interest.
   * In particular it aims at sorting the Operating Point according to the given criteria,
   * represented by the Evaluator object.
   * Since it is possible that the Evaluator produces the same value for two different
   * Operating Points, the underlying container is a std::multimap.
   * All the Operating Points are sorted in an ascending order.
   */
  template< class OperatingPoint, FieldComposer composer, class ...Fields >
  class View
  {

      /**
       * @brief Explicit definition of the Operating Point list
       */
      using OperatingPointList = typename Knowledge<OperatingPoint>::OperatingPointList;


      /**
       * @brief Explicit definition of an iterator of the Operating Point list
       */
      using OperatingPointListIt = typename OperatingPointList::const_iterator;


    public:

      /**
       * @brief Explicit definition of a pointer to the Operating Point
       */
      using OperatingPointPtr = typename Knowledge<OperatingPoint>::OperatingPointPtr;


      /**
       * @brief Declarion of the Evaluator type used to evaluate an Operating Point
       */
      using ViewEvaluator = Evaluator< OperatingPoint, composer, Fields... >;


      /**
       * @brief The type of the evaluated number by the target Evaluator
       */
      using value_type = typename ViewEvaluator::value_type;


      /**
       * @brief Definition of the container used to sort the Operating Point
       */
      using Container = std::multimap< value_type, OperatingPointPtr >;


      /**
       * @brief Definition of the stream of Operating Point
       *
       * @details
       * This container is used as interface from the other component of the framework
       * to select slices of Operating Point.
       */
      using OPStream = typename Knowledge<OperatingPoint>::OPStream;


      /**
       * @brief Default constructor
       *
       * @details
       * This is a trivial constructor, which evaluate all the Operating Point to zero
       */
      View( void )
      {
        evaluate = [] (const OperatingPointPtr & target_op)
        {
          return static_cast<value_type>(0);
        };
      }


      /**
       * @brief Constructor of the class
       *
       * @param [in] ...values The list of fields that define the evaluation criteria
       *
       * @details
       * This is the actual constructor that exploit the Evaluator to evaluate
       * a given Operating Point
       */
      View(Fields ...values)
      {
        evaluate = [values...] (const OperatingPointPtr & target_op)
        {
          return ViewEvaluator::evaluate(target_op, values...);
        };
      }


      /******************************************************************
       *  METHODS THAT MANIPULATE THE CONTAINER
       ******************************************************************/


      /**
       * @brief Add Operating Points to the view
       *
       * @param [in] begin Iterator to the begin of the Operating Point list
       * @param [in] end Iterator to the end of the Operating Point list
       *
       * @details
       * This method is meant to be used to initialize the view with all the Operating Points
       * in the knowledge base.
       */
      inline void add( const OperatingPointListIt& begin, const OperatingPointListIt& end )
      {
        for ( auto it = begin; it != end; ++it)
        {
          sorted_knowledge.emplace(evaluate(it->second), it->second);
        }
      }


      /**
       * @brief Add a single Operating Point to the view
       *
       * @param [in] new_op A shared pointer to the Operating Point to add
       *
       * @note
       * For perfomance reason, this method doesn't check if the new Operating Point
       * is already in view.
       */
      inline void add( const OperatingPointPtr& new_op )
      {
        sorted_knowledge.emplace(evaluate(new_op), new_op);
      }


      /**
       * @brief Remove the target Operating Point
       *
       * @param [in] target_op A shared pointer to the Operating Point to remove
       *
       * @note
       * This method removes only a single Operating Point from the view.
       */
      void remove( const OperatingPointPtr& target_op )
      {

        // get all the Operating Point with the same value as the target Operating Point
        const value_type target_op_value = evaluate(target_op);
        const auto available_range = sorted_knowledge.equal_range(target_op_value);

        // iterate through them, searching for the target Operating Point to erase
        for ( auto it = available_range.first; it != available_range.second; ++it)
        {
          if (it->second == target_op)
          {
            sorted_knowledge.erase(it);
            break;
          }
        }
      }


      /**
       * @brief Remove all the Operating Points from the view
       */
      inline void clear( void )
      {
        sorted_knowledge.clear();
      }


      /******************************************************************
       *  UTILITY METHODS
       ******************************************************************/


      /**
       * @brief Retrieve the size of the view
       *
       * @return The number of Operating Points in the view
       */
      inline std::size_t size( void ) const
      {
        return sorted_knowledge.size();
      }


      /**
       * @brief Test whether the view is empty
       *
       * @return True, if there are no Operating Points in the view
       */
      inline bool empty( void ) const
      {
        return sorted_knowledge.empty();
      }


      /**
       * @brief Evaluate a target Operating Point
       *
       * @param [in] target_op A shared pointer to the Operating Point to evaluate
       *
       * @return The value of the Operating Point
       */
      inline value_type evaluate_op( const OperatingPointPtr target_op ) const
      {
        return evaluate(target_op);
      }


      /******************************************************************
       *  SLICING METHODS
       ******************************************************************/


      /**
       * @brief Retrieve the front of the container
       *
       * @return A shared pointer to the first Operating Point of the view
       *
       * @details
       * If there is no Operating Points in the view, it returns a null pointer
       */
      inline OperatingPointPtr front( void ) const
      {
        return !sorted_knowledge.empty() ? sorted_knowledge.begin()->second : OperatingPointPtr{};
      }


      /**
       * @brief Retrieve the back of the container
       *
       * @return A shared pointer to the last Operating Point of the view
       *
       * @details
       * If there is no Operating Points in the view, it returns a null pointer
       */
      inline OperatingPointPtr back( void ) const
      {
        return !sorted_knowledge.empty() ? std::prev(sorted_knowledge.end())->second : OperatingPointPtr{};
      }


      /**
       * @brief Retrieve the sorted knowledge base
       *
       * @return A OPStream with all the Operating Points
       *
       * @details
       * This method is used to retrieve all the Operating Points in the view, sorted
       * in an ascended order, according to the evaluation criteria.
       */
      OPStream range( void ) const
      {
        OPStream result;

        for ( const auto& pair : sorted_knowledge )
        {
          result.emplace_back(pair.second);
        }

        return result;
      }


      /**
       * @brief Retrieve a slice of the knowledge base
       *
       * @param [in] a The first value which identifies the slice
       * @param [in] b The last value which identifies the slice
       *
       * @return A OPStream that contains all the Operating Points from a to b
       *
       * @details
       * This method aims at selecting a subset of the Operating Point in the view
       * that evaluate from value a (included) to value b (included).
       */
      OPStream range( const value_type a, const value_type b ) const
      {
        OPStream result;

        // make sure to find the minimum and maximum value
        const value_type min = std::min(a, b);
        const value_type max = std::max(a, b);

        // get the iterators that defines the slice of Operating Points
        const auto start_it = sorted_knowledge.lower_bound(min);
        const auto stop_it = sorted_knowledge.upper_bound(max);

        // populte the stream of Operating Points
        for ( auto it = start_it; it != stop_it; ++it )
        {
          result.emplace_back(it->second);
        }

        return result;
      }


    protected:


      /**
       * @brief A lambda used to evaluate an Operating Point
       */
      std::function<value_type(const OperatingPointPtr&)> evaluate;


    private:


      /**
       * @brief The container, used to store all the Operating Point of the view
       */
      Container sorted_knowledge;
  };


}

#endif // MARGOT_VIEW_HDR
