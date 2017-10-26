/* core/rank.hpp
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

#ifndef MARGOT_RANK_HDR
#define MARGOT_RANK_HDR

#include <memory>
#include <cassert>
#include <functional>
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>
#include <iostream>

#include "margot/operating_point.hpp"
#include "margot/view.hpp"
#include "margot/knowledge_base.hpp"
#include "margot/debug.hpp"

namespace margot
{

  /**
   * @brief The interface of the actual Rank implementation
   *
   * @tparam OperatingPoint The type of the target Operating Point
   *
   * @details
   * The rank value represents the objective function that must be maximized or minimized,
   * according to the application requirements.
   * This class is the interface that hides all the details about the currrent definition
   * of rank. The framework is only interested on managing the Operating Points, e.g. finding
   * the best one. It doesn't care about the actual definition.
   * Moreover, the framework is interested on the rank of only the valid Operating Points.
   * For this reason, the Operating Point stored in the rank are typically fewer than the
   * ones in the knowledge.
   */
  template< class OperatingPoint >
  class RankInterface
  {


      // statically check the template argument
      static_assert(traits::is_operating_point<OperatingPoint>::value,
                    "Error: the Rank handles object with is_operating_point trait");


    public:


      /**
       * @brief Explicit definition to an Operating Point pointer
       */
      using OperatingPointPtr = typename Knowledge<OperatingPoint>::OperatingPointPtr;


      /**
       * @brief Definition of the stream of Operating Point
       *
       * @details
       * This container is used as interface from the other component to find
       * the best OperatingPoint from the container
       */
      using OPStream = typename Knowledge<OperatingPoint>::OPStream;


      /*
       * @brief Definition of pointer to this rank interface
       */
      using RankInterfacePtr = std::shared_ptr< RankInterface<OperatingPoint> >;


      /**
       * @brief Creates a pseudo-copy of the underlying rank
       *
       * @return A shared pointer to the sibling rank
       *
       * @details
       * This method creates a copy of the actual rank, which have the
       * same features, but it lacks any relation with the managed OPs.
       * This method is used to interact with the Data-Aware AS-RTM.
       */
      virtual RankInterfacePtr create_sibling( void ) const = 0;


      /**
       * @brief Add a stream of Operating Points to the rank
       *
       * @param [in] new_ops a OPStream of Operating Point to be added to the rank
       *
       * @details
       * If a set of Operating Points becames available at RunTime, this method is
       * used to add them to the set of valid ones, represented by the rank.
       */
      virtual void add( const OPStream& new_ops ) = 0;


      /**
       * @brief Remove a stream of Operating Point from the rank
       *
       * @param [in] ops A OPStream of Operating Point to be removed from the rank
       *
       * @details
       * If a set of Operating Points becames not valid at RunTime, this method is
       * used to remove them from the set of valid ones, represented by the rank.
       */
      virtual void remove( const OPStream& ops ) = 0;


      /**
       * @brief Removes all the Operating Points from the Rank
       */
      virtual void clear( void ) = 0;


      /**
       * @brief Retrieves the best Operating Point from the rank
       *
       * @return A shared pointer to the best Operating Point from the rank
       */
      virtual OperatingPointPtr best( void ) const = 0;


      /**
       * @brief Evaluates a stream of Operating Points
       *
       * @param [in] ops A OPStream of Operating Points that must be evaluated
       *
       * @return A shared pointer to the best Operating Point found
       *
       * @details
       * This method does not find the best Operating Point between the ones stored
       * in the rank, but between the ones passed as arguments
       *
       * @note
       * The size of the parameter must be greater than one.
       */
      virtual OperatingPointPtr best( const OPStream& ops ) const = 0;


      /**
       * @brief Retrieves all the valid Operating Points
       *
       * @return A OPStream of the valid Operating Points
       */
      virtual OPStream to_stream( void ) const = 0;


      /**
       * @brief Print the status of the rank, for debug purpose
       */
      virtual void dump( const std::string& prefix ) const = 0;


      /**
       * @brief Virtual destructor
       */
      virtual ~RankInterface( void ) {}

  };




  /******************************************************************
   *  HELPER STRUCT TO HANDLE THE RANK OBJECTIVE
   ******************************************************************/


  /**
   * @brief Helper struct, to define the correct comparison function
   *
   * @tparam T The type of the evaluated value from the Operating Point
   * @tparam object The objective of the rank, i.e. minimize or maximize
   *
   * @see RankObjective
   *
   * @details
   * This struct exposes a unified interface to test which is the best Operating
   * Point between two alternatives.
   * For example, if the user would like to maximize the rank, than we use the
   * greater comparison function. Otherwise we must use the lower comparison
   * function.
   * This struct takes advantage of partial specialization to select the correct
   * comparison function. Since this class represents the general case, you should
   * never be able to use this struct.
   */
  template< typename T, RankObjective objective >
  struct rank_compare;


  /**
   * @brief Specialization of the helper struct, when we want to maximize the rank
   *
   * @tparam T The type of the evaluated value from the Operating Point
   *
   * @see rank_compare
   */
  template< typename T >
  struct rank_compare<T, RankObjective::MAXIMIZE>
  {


    /**
     * @brief Check whether the lhs value is better than the rhs value
     *
     * @param [in] lhs The rank value of the left hand side Operating Point
     * @param [in] rhs The rank value of the right hand side Operating Point
     *
     * @return True, if lhs is greater than rhs
     */
    inline static bool best( const T lhs, const T rhs )
    {
      return lhs > rhs;
    }

  };


  /**
   * @brief Specialization of the helper struct, when we want to minimize the rank
   *
   * @tparam T The type of the evaluated value from the Operating Point
   *
   * @see rank_compare
   */
  template< typename T >
  struct rank_compare<T, RankObjective::MINIMIZE>
  {


    /**
     * @brief Check whether the lhs value is better than the rhs value
     *
     * @param [in] lhs The rank value of the left hand side Operating Point
     * @param [in] rhs The rank value of the right hand side Operating Point
     *
     * @return True, if lhs is lower than rhs
     */
    inline static bool best( const T lhs, const T rhs )
    {
      return lhs < rhs;
    }

  };




  /******************************************************************
   *  ACTUAL IMPLEMENTATION OF THE RANK
   ******************************************************************/


  /**
   * @brief The implementation of the RankInterface
   *
   * @tparam OperatingPoint The type of the target Operating Point
   * @tparam obective The objective function of the rank (i.e. maximizing or minimizing)
   * @tparam composer The way of combining the fields that ranks an Operating Point
   * @tparam ...Fields The list of fields of interest to rank an Operating Point
   *
   * @details
   * The rank is very similar to a View, which means that it must evaluate all the Operating Point
   * according to the application requirements.
   * The main difference is that the Rank consider only the valid Operating Points.
   *
   * This class is implemented as an extension of a View, implementing all the methods required
   * by the RankInterface.
   */
  template< class OperatingPoint, RankObjective objective, FieldComposer composer, class ...Fields >
  class Rank: public View< OperatingPoint, composer, Fields... >, public RankInterface<OperatingPoint>
  {


      // statically check the template argument
      static_assert(traits::is_operating_point<OperatingPoint>::value,
                    "Error: the Rank handles object with is_operating_point trait");


      /**
       * @brief Explicit definition of the View that defines the rank
       */
      using MyView =  View< OperatingPoint, composer, Fields... >;


      /**
       * @brief Explicit definition of the type of the value computed by the view
       */
      using rank_type = typename MyView::value_type;


      /**
       * @brief Definition of the target rank_composer, according to application requirements
       */
      using RankComparator = rank_compare< rank_type, objective >;


    public:


      /**
       * @brief Explicit definition to an Operating Point pointer
       */
      using OperatingPointPtr = typename RankInterface<OperatingPoint>::OperatingPointPtr;


      /**
       * @brief Definition of the stream of Operating Point
       *
       * @details
       * This container is used as interface from the other component to find
       * the best OperatingPoint from the container
       */
      using OPStream = typename RankInterface<OperatingPoint>::OPStream;


      /**
       * @brief Explicit definition of the sibling rank pointer
       */
      using RankInterfacePtr = typename RankInterface<OperatingPoint>::RankInterfacePtr;


      /**
       * @brief Default constructor
       *
       * @see View
       */
      Rank( void ): MyView()
      {}


      /**
       * @brief Constructor of the class
       *
       * @see View
       */
      Rank(Fields ...values): MyView(values...)
      {}


      /**
       * @brief Creates a pseudo-copy of the underlying rank
       *
       * @return A shared pointer to the sibling rank
       *
       * @details
       * This method creates a copy of the actual rank, which have the
       * same features, but it lacks any relation with the managed OPs.
       * This method is used to interact with the Data-Aware AS-RTM.
       */
      RankInterfacePtr create_sibling( void ) const
      {
        // since we don't have anymore the information regarding the coefficients of
        // the rank values, we have to create a rank object with the trivial constructor
        // and then replace the evaluator with the actual evaluating function
        std::shared_ptr< Rank<OperatingPoint,objective,composer,Fields...> > sibling;
        sibling.reset(new Rank<OperatingPoint,objective,composer,Fields...>());
        sibling->MyView::evaluate = MyView::evaluate;

        // now we have to perform a pointer conversion
        return std::static_pointer_cast<typename  RankInterfacePtr::element_type>(sibling);
      }


      /**
       * @brief Implementation of the RankInterface add method
       *
       * @see RankInterface
       */
      void add( const OPStream& new_ops )
      {
        for ( const auto& op : new_ops )
        {
          MyView::add(op);
        }
      }


      /**
       * @brief Implementation of the RankInterface remove method
       *
       * @see RankInterface
       */
      void remove( const OPStream& ops )
      {
        for ( const auto& op : ops )
        {
          MyView::remove(op);
        }
      }


      /**
       * @brief Implementation of the RankInterface clear method
       *
       * @see RankInterface
       */
      void clear( void )
      {
        MyView::clear();
      }


      /**
       * @brief Implementation of the RankInterface best method
       *
       * @see RankInterface
       *
       * @details
       * In the current implementation, this method do not check if there are not valid
       * Operating Points, i.e. the View container is empty.
       * It relies on the front and back method of the View to return a nullptr if there
       * are no Operating Points in the view.
       */
      OperatingPointPtr best( void ) const
      {
        return objective == RankObjective::MINIMIZE ? MyView::front() : MyView::back();
      }


      /**
       * @brief Implementation of the RankInterface best method
       *
       * @see RankInterface
       *
       * @details
       * This method takes advantage of the std::min_element method to retrieve the
       * best Operating Point from the stream, deferencing an iterator.
       * For performance reason, this method checks if stream is empty only in Debug
       * mode.
       * For this reason, retrieving the best Operating Point from an empty stream is
       * considered undefined behavior.
       */
      OperatingPointPtr best( const OPStream& ops ) const
      {
        assert(!ops.empty() && "Error: Rank is evaluating an empty Operating Point Stream");
        return *std::min_element(ops.begin(), ops.end(),
                                 [&](const OperatingPointPtr & lhs, const OperatingPointPtr & rhs)
        {
          return RankComparator::best(MyView::evaluate(lhs), MyView::evaluate(rhs));
        });
      }


      /**
       * @brief Implementation of the RankInterface to_stream method
       *
       * @see RankInterface
       */
      OPStream to_stream( void ) const
      {
        return MyView::range();
      }


      /**
       * @brief Print the status of the rank, for debug purpose
       */
      void dump( const std::string& prefix ) const;

  };


  template< class OperatingPoint, RankObjective objective, FieldComposer composer, class ...Fields >
  void Rank<OperatingPoint,objective,composer,Fields...>::dump(const std::string &prefix) const
  {
    const std::string direction = objective == RankObjective::MAXIMIZE ? "Maximize" : "Minimize";
    std::cout << prefix << " Rank objective: " << direction << std::endl;


    if (MyView::empty())
    {
      std::cout << prefix << std::endl;
      std::cout << prefix << " There are no valid Operating Points" << std::endl;
    }
    else
    {
      for( const auto op_pair : MyView::sorted_knowledge )
      {
        std::cout << prefix << std::endl;
        print_conf_with_value<OperatingPoint,rank_type>(op_pair.second, op_pair.first, prefix, "Rank");
        std::cout << prefix << std::endl;
      }
    }
  }

}

#endif // MARGOT_RANK_HDR
