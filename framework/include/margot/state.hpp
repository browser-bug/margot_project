/* core/state.hpp
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

#ifndef MARGOT_STATE_HDR
#define MARGOT_STATE_HDR

#include <cstddef>
#include <memory>
#include <map>
#include <vector>
#include <cassert>
#include <string>
#include <iostream>

#include "margot/operating_point.hpp"
#include "margot/knowledge_adaptor.hpp"
#include "margot/knowledge_base.hpp"
#include "margot/goal.hpp"
#include "margot/enums.hpp"
#include "margot/constraint.hpp"
#include "margot/rank.hpp"
#include "margot/debug.hpp"

namespace margot
{

  /**
   * @brief This class represents a constrained multi-objective optimization problem
   *
   * @tparam OperatingPoint The type which defines the Operating Point characteristics
   * @tparam priority_type The type used to represents the priority of a constraint
   * @tparam error_coef_type The type used to compute the error coefficient of the application knowledge
   *
   * @details
   * This class is the core of the mARGOt framework since it aims at solving the optimization problem,
   * which is formulated as follows:
   *
   *         minimize/minimize f(OperatingPoint)
   *
   *           s.t.     c1(OperatingPoint) § goal1
   *                    c2(OperatingPoint) § goal2
   *                          ...            ...
   *
   * Where f(OperatingPoint) represents the objective funcion to maxime or minimize, cn extracts a numeric
   * value from an Operating Point and goaln is the goal to achieve to satisfy the n-th constraint.
   * This class provides all the required methods to alter the formulation of the problem, to alter the
   * application knowledge, to take into account runtime information and to solve it.
   *
   * From the implementation points of ciew, a state uses constraints to filter all the Operating Point in
   * the application knowledge. In particular, each constraint keeps track internally of all the Operating
   * Point valid for higher priority constraints, but not valid for the considered constraint.
   * If an Operating Point is valid for all the constraints, it is inserted in the rank container, which
   * evaluate all the valid Operating Point according to the objective function.
   *
   * This class has two main task: keep a consistent representation of the problem and select the most
   * suitable configuration in an efficient way.
   */
  template< class OperatingPoint, typename priority_type = int, typename error_coef_type = float >
  class State
  {


      // statically check the template argument
      static_assert(traits::is_operating_point<OperatingPoint>::value,
                    "Error: the State handles object with is_operating_point trait");


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


      /**
       * @brief Definition of a pointer to a generic constraint
       */
      using ConstraintPtr = std::shared_ptr< ConstraintHandler< OperatingPoint, error_coef_type > >;


      /**
       * @brief Definitionof a pointer to the rank interface
       */
      using RankPtr = std::shared_ptr< RankInterface< OperatingPoint > >;


      /**
       * @brief Aliasing of the structure used to store the set of constraint
       */
      using ConstraintStack = std::map< priority_type, ConstraintPtr >;


      /**
       * @brief Default constructor
       *
       * @details
       * By default a state defines the objective function to minimize the average value of
       * the first software knob. We have chosen this definition because we are sure that the
       * Operating Point will have at least one software knob.
       */
      State( void ): problem_is_changed(true)
      {
        // we are just interested in one field that, for sure, is available: the first software knob
        using dummy_field = OPField< OperatingPointSegments::SOFTWARE_KNOBS, BoundType::LOWER, 0, 0>;

        // create a dummy rank, just to have a definition of best
        // and to have a container for all the Operating Points
        rank.reset( new Rank< OperatingPoint, RankObjective::MINIMIZE, FieldComposer::SIMPLE, dummy_field >( 1.0f ) );
      }




      /******************************************************************
       *  METHODS TO INTERACT WITH THE DATA-AWARE AS-RTM
       ******************************************************************/


      /**
       * @brief Creates a pseudo-copy of the state
       *
       * @return A State object with the same optimization problem
       *
       * @details
       * This method creates a state, which is independent with respect to the
       * current one, but has the same optimization problem structure.
       * Since this state will manage different Operating Points, it does not
       * copy any of them, from this state.
       */
      State create_sibling( void ) const
      {
        // declare the cloned state
        State sibling;

        // set the rank function as this state
        sibling.rank = rank->create_sibling();


        // replicate the constraint structure
        for ( const auto& constraint_pair : constraints )
        {
          sibling.constraints.emplace(constraint_pair.first, constraint_pair.second->create_sibling());
        }


        // return the created sibling
        return sibling;
      }




      /******************************************************************
       *  CONSTRAINTS MANAGEMENT
       ******************************************************************/


      /**
       * @brief Add a constraint in the optimization problem
       *
       * @tparam segment The enumerator value of the target segment of the Operating Point
       * @tparam field_index The index of the target field within the target segment
       * @tparam sigma The confidence of the constraints, i.e. the number of times the standard deviation is taken into account
       * @tparam ConstraintGoal The type of the constraint's goal
       *
       * @param [in] goal_value The goal that represents the constraint
       * @param [in] priority The priority of the new constraint
       * @param [in] kb The knowledge base of the application
       * @param [in] adaptor The runtime information provider
       *
       * @see State
       *
       * @details
       * This method insert and initialize a new constraint in the optimization problem. To initialize
       * the constraint we have to populate the view of the constraint using the application knowledge,
       * update the internal structure of the Operating Points and set the field adaptor for the constraint.
       * Adding a constraint is not a cheap operation. It is better to do so in the "initialization" of the
       * application, for performance reason.
       * The priority is used as unique identifier for a constraint. If we are adding a constraint with the
       * same priority of a previous one, the former will overwrite the latter
       */
      template< OperatingPointSegments segment, std::size_t field_index, int sigma, class ConstraintGoal >
      void add_constraint( const ConstraintGoal& goal_value,
                           const priority_type priority,
                           const Knowledge<OperatingPoint>& kb,
                           const KnowledgeAdaptor<OperatingPoint, error_coef_type>& adaptor )
      {
        // this constraint is going to invalidate some Operating Points which are not blocked
        // by higher priority constraint.
        // To insert the constraint we have to:
        //  - get all the Operating Points from the rank and lower priority constraints
        //  - create the new constraint
        //  - let the new constraint to block the invalid Operating Points
        //  - remove those point from lower constraints and the rank

        // get the Operating Points from the rank
        OPStream available_ops = rank->to_stream();

        // get the range of constraints less important with respect to the current one
        // in case we are replacing a constraint, we need to consider it
        const auto start_it = constraints.lower_bound(priority);
        const auto end_it = constraints.end();

        // append the ones blocked by the lower constraints
        for ( auto it = start_it; it != end_it; ++it )
        {
          it->second->append_to(available_ops);
        }

        // create the new constraint
        auto new_constraint = ConstraintPtr( new Constraint<OperatingPoint,
                                             segment,
                                             field_index,
                                             sigma,
                                             ConstraintGoal,
                                             error_coef_type>(goal_value) );
        assert(new_constraint && "Error: the State is unable to create a new constraint");
        new_constraint->set(kb);
        new_constraint->set_field_adaptor(adaptor);


        // insert (or replace ) the new constraint in the deck of constraints
        constraints[priority] =  new_constraint;

        // feed the new constraint with the available Operating Points
        OPStream filtered_ops;
        new_constraint->filter_initialize(available_ops, filtered_ops);

        // if the constraint do not invalidate any Operating Points, it is ok
        if (filtered_ops.empty())
        {
          return;
        }

        // if we have reached this point, the best Operating Point might be
        // different, therefore we need to solve again the problem
        problem_is_changed = true;

        // remove from the lower priority constraints the blocked OPs
        const auto lower_it = constraints.upper_bound(priority);

        for ( auto it = lower_it; it != end_it; ++it )
        {
          OPStream ops_to_be_removed;
          ops_to_be_removed.swap(filtered_ops);
          lower_it->second->filter_remove(ops_to_be_removed, filtered_ops);
        }

        // remove the blocked OPs also from the rank
        rank->remove(filtered_ops);
      }


      /**
       * @brief Remove a constraint from the optimization problem
       *
       * @param [in] priority The priority of the target constraint to remove
       *
       * @see State
       *
       * @details
       * This method remove a constraint from the optimization problem and change the
       * internal structure according to the Operating Points blocked by the target
       * constraint. This may be a not cheap operation.
       */
      void remove_constraint( const priority_type priority )
      {
        // search for the constraint
        const auto constraint_it = constraints.find(priority);

        // get the reference to the end of the constraint stack
        const auto end_it = constraints.end();

        // check if the constraint exists
        if ( constraint_it != end_it )
        {
          // get all the Operating Point blocked from the constraint
          OPStream available_ops = constraint_it->second->to_stream();

          // check if the constraint was blocking some Operating Points
          if (!available_ops.empty())
          {
            // insert them to the lower priority constraints
            OPStream ops_to_add;

            for ( auto it = std::next(constraint_it); it != end_it; ++it )
            {
              ops_to_add.swap(available_ops);
              it->second->filter_add(ops_to_add, available_ops);
            }

            // insert the remainder of them in the rank
            rank->add(available_ops);

            // if we have reached this point, the best Operating Point might be
            // different, therefore we need to solve again the problem
            problem_is_changed = true;
          }

          // eventually, erase the constraint
          constraints.erase(constraint_it);
        }
      }




      /******************************************************************
       *  RANK MANAGEMENT
       ******************************************************************/


      /**
       * @brief Set the rank of the active state, i.e. its objective function
       *
       * @tparam objective Enumerator to tell if we want to maximize or minimize the objective function
       * @tparam composer Enumerator to select the way the objective function is composed
       * @tparam ...Fields The list of fields that compose the objective function
       *
       * @param [in] ...values The coefficients of each field that compose the objective function
       *
       * @see Rank
       *
       * @details
       * This method store in a stream all the valid Operating Points, replace the previous rank
       * function and insert again the valid Operating Points.
       */
      template< RankObjective objective, FieldComposer composer, class ...Fields >
      void set_rank( Fields ...values )
      {
        // get all the valid Operating Points
        OPStream valid_ops = rank->to_stream();

        // create a new rank or replace a previous rank
        rank.reset( new Rank< OperatingPoint, objective, composer, Fields... >( values... ) );

        // check if there are valid ops
        if (!valid_ops.empty())
        {
          // feed the rank with valid ops
          rank->add(valid_ops);

          // we have to solve again the optimization problem
          problem_is_changed = true;
        }

      }




      /******************************************************************
       *  UTILITY METHODS TO UPDATE THE APPLICATION KNWOLEDGE
       ******************************************************************/


      /**
       * @brief Add a stream of Operating Points in the state
       *
       * @param [in] new_ops A OPStream containing the new Operating Points
       *
       * @details
       * This methods at first update the view of all the constraint that compose
       * the optimization problem. In a second step evaluates all the new Operating
       * Points.
       */
      void add_operating_points( const OPStream& new_ops )
      {
        // update the constraints view
        for ( auto& constraint_pair : constraints )
        {
          // add the OP to the view of the constraint
          for ( const auto& op : new_ops )
          {
            constraint_pair.second->add(op);
          }
        }

        // filter them throught the constraints
        OPStream ops_to_add;
        OPStream new_ops_shrinkable = new_ops;

        for ( auto& constraint_pair : constraints )
        {
          ops_to_add.swap(new_ops_shrinkable);
          constraint_pair.second->filter_add( ops_to_add, new_ops_shrinkable );
        }

        // add them to the rank
        rank->add(new_ops_shrinkable);

        // we have to solve again the optimization problem
        problem_is_changed = true;
      }


      /**
       * @brief Remove a stream of Operating Points
       *
       * @param [in] ops A OPStream with all the Operating Points to remove
       *
       * @details
       * This methods removes the Operating Points from the constraints internal container of
       * the blocked Operating Points and from the constraints views.
       */
      void remove_operating_points( const OPStream& ops )
      {
        // update the constraints view
        for ( auto& constraint_pair : constraints )
        {
          // remove the OP from the view of the constraint and the blocked OPs
          for ( const auto& op : ops )
          {
            constraint_pair.second->remove(op);
          }
        }

        // remove them from the rank
        rank->remove(ops);

        // we have to solve again the optimization problem
        problem_is_changed = true;
      }


      /**
       * @brief Initialize the constraint with a new knowledge base
       *
       * @param [in] kb The application knowledge
       *
       * @details
       * This method sets all the Operating Point points in the knowledge base
       * in the views of all the contraints and then filter all the Operating Points
       * through the list of constraints and the rank.
       */
      void set_knowledge_base( const Knowledge<OperatingPoint>& kb )
      {
        // set the views and clear the blocked ops from all the constraints
        for ( auto& constraint_pair : constraints )
        {
          constraint_pair.second->set(kb);
          constraint_pair.second->clear();
        }

        // clear the rank
        rank->clear();

        // get all the available Operating Points from the new knowledge base
        OPStream available_ops = kb.to_stream();

        // feed them to the constraints
        OPStream ops_to_add;

        for ( auto& constraint_pair : constraints )
        {
          ops_to_add.swap(available_ops);
          constraint_pair.second->filter_add(ops_to_add, available_ops);
        }

        // insert them in the rank
        rank->add(available_ops);

        // we have to solve again the optimization problem
        problem_is_changed = true;
      }


      /**
       * @brief Update all the constraints with new runtime information providers
       *
       * @param [in] adaptor The new runtime information provider
       */
      void set_knowledge_adaptor( const KnowledgeAdaptor<OperatingPoint, error_coef_type>& adaptor )
      {
        // set the adaptor for the constraint
        for ( auto& constraint_pair : constraints )
        {
          constraint_pair.second->set_field_adaptor(adaptor);
        }

        // we have to solve again the optimization problem
        problem_is_changed = true;
      }




      /******************************************************************
       *  METHOD TO ACTUALLY SOLVE THE OPTIMIZATION PROBLEM
       ******************************************************************/


      /**
       * @brief This method solve the optimization problem
       *
       * @return The OperatinPointPtr which represents the most suitable configuration
       *
       * @details
       * At first this method update each constraint to see if there are changes in the
       * goal values or in the execution environment.
       * Then finds the best Operating Point according to the application requirements.
       *
       * Internally, it uses memoization-like mechanism in order to avoid any computation
       * if the situation is not changed from the last time we have found the most suitable
       * configuration.
       */
      OperatingPointPtr get_best_operating_point( void )
      {
        // first update the internal structure to see if there are differences
        update();

        // check if we can avoid to solve again the problem
        if (!problem_is_changed)
        {
          return best_operating_point_found;
        }

        // ok, something is changed, we have to actually solve the problem.
        // But, the new solution will be the best one until something change.
        problem_is_changed = false;

        // try with the best according to the rank
        OperatingPointPtr best_op = rank->best();

        if (best_op)
        {
          best_operating_point_found = best_op;
          return best_operating_point_found; // the actual valid best op
        }

        // in this case, either there are no Operating Points, or all of them are invalid

        // make sure that we have some constraints, otherwise we have no Operating Points
        if (constraints.empty())
        {
          best_operating_point_found = best_op;
          return best_operating_point_found; // a nullptr
        }

        // ok, let's start relaxing constraints
        auto constraint_pair_it = constraints.end();
        const auto first_constraint_pair_it = constraints.begin();

        for ( constraint_pair_it = std::prev(constraint_pair_it); constraint_pair_it != first_constraint_pair_it;
              constraint_pair_it = std::prev(constraint_pair_it) )
        {
          // get the ops closest to the constraint
          OPStream best_ops = constraint_pair_it->second->get_closest();

          // if we have found some ops, we can work with them
          if (!best_ops.empty())
          {
            best_operating_point_found = get_best_from_stream(best_ops, constraint_pair_it );
            return best_operating_point_found; // the actual invalid best op
          }
        }

        // if we have reached this point, we have to rely on the first constraint
        OPStream best_ops = first_constraint_pair_it->second->get_closest();

        // if we have found some ops, we can work with them
        if (!best_ops.empty())
        {
          best_operating_point_found = get_best_from_stream(best_ops, first_constraint_pair_it );
          return best_operating_point_found; // the actual invalid best op
        }

        // if have reached this point, there are no Operating Points
        best_operating_point_found = OperatingPointPtr{};
        return best_operating_point_found;
      }


      /**
       * @brief Dump the status of the state for debug purpose
       */
      void dump( const std::string& prefix ) const;


    private:


      /**
       * @brief Updates all the constraints in the optimization problem
       */
      void update( void )
      {
        // get a reference to the first constraint
        const auto first_constraint_pair_it = constraints.begin();
        const auto end_constraint_pair_it = constraints.end();

        // loop over the constraint, starting from the one at high priority
        OPStream ops_to_be_added;
        OPStream ops_to_be_removed;

        for ( auto it = first_constraint_pair_it; it != end_constraint_pair_it; ++it )
        {
          // update the current constraint
          it->second->update(ops_to_be_removed, ops_to_be_added);

          // check if we need to remove Operating Points
          if (!ops_to_be_removed.empty())
          {
            // we have to prune the already blocked ops from high priority constraints
            OPStream ops_to_be_removed_for_real;

            for ( auto higher_it = first_constraint_pair_it; higher_it != it; ++higher_it )
            {
              ops_to_be_removed_for_real.swap(ops_to_be_removed);
              higher_it->second->remove_blocked_ops_from(ops_to_be_removed_for_real, ops_to_be_removed);
            }

            // check if we need to actually change the structure
            if (!ops_to_be_removed.empty())
            {
              // make a copy of the Operating Points to be removed
              // because we need to remove them later from lower priority constraints
              ops_to_be_removed_for_real = ops_to_be_removed;

              // now we are able to block them in the current constraint
              OPStream remainder_ops;
              it->second->filter_add(ops_to_be_removed, remainder_ops);

              // this is because we know for sure that the current constraint blocks that Operating Points
              assert(remainder_ops.empty() && "Error: on the update something went wrong, internal error");

              // now we have to remove them from lower constraints
              for ( auto lower_it = std::next(it); lower_it != end_constraint_pair_it; ++lower_it )
              {
                remainder_ops.swap(ops_to_be_removed_for_real);
                lower_it->second->filter_remove(remainder_ops, ops_to_be_removed_for_real);
              }

              // we have to remove them also from the rank
              rank->remove(ops_to_be_removed_for_real);

              // we have to solve again the optimization problem
              problem_is_changed = true;
            }

          }

          // check if more Operating Points are available
          if (!ops_to_be_added.empty())
          {
            // loop over the lower constraint to add Operating Points
            OPStream op_leftover;

            for ( auto lower_it = std::next(it); lower_it != end_constraint_pair_it; ++lower_it )
            {
              op_leftover.swap(ops_to_be_added);
              lower_it->second->filter_add(op_leftover, ops_to_be_added);
            }

            // insert them into the rank
            rank->add(ops_to_be_added);

            // we have to solve again the optimization problem
            problem_is_changed = true;
          }
        }
      }


      /**
       * @brief Find the most suitable configuration from a stream of Operating Point
       *
       * @param [in] input_ops The OPStream of Operating Point to evaluate
       * @param [in] constraint_it The iterator of the relaxed constraint
       *
       * @details
       * If there are no valid Operating Points, we start to relax the contraints, starting with the ones
       * with low priority. Once we found a constraint that is blocking some Operating Points, we get
       * the closest ones to achieve its goal.
       * From these Operating Points, this methods find the most suitable one.
       */
      OperatingPointPtr get_best_from_stream( OPStream& input_ops, const typename ConstraintStack::iterator& constraint_it ) const
      {
        // if there is only one Operating Point it's good
        if (input_ops.size() == 1)
        {
          return input_ops.front();
        }

        // otherwise, we have to narrow them down with the other constraint (lower priority)
        const auto end_it = constraints.end();
        OPStream narrowed_down_ops;

        for ( auto lower_it = std::next(constraint_it); lower_it != end_it; ++lower_it )
        {
          // narrow them down using the considered constraint
          narrowed_down_ops = lower_it->second->narrow(input_ops);
          input_ops.swap(narrowed_down_ops);

          // if there is only one OP, it is done
          if (input_ops.size() == 1)
          {
            return input_ops.front();
          }
        }

        // if we have reached this point, we have to use the rank to select
        // the best invalid Operating Point.
        return rank->best(input_ops);
      }


      /**
       * @brief The list of constraint of the optimization problem
       */
      ConstraintStack constraints;


      /**
       * @brief The objective function of the optimization problem
       */
      RankPtr rank;


      /**
       * @brief Varable, used to keep track of changes in the state
       */
      bool problem_is_changed;


      /**
       * @brief Pointer to the most suitable Operating Point found
       */
      OperatingPointPtr best_operating_point_found;

  };



  template< class OperatingPoint, typename priority_type, typename error_coef_type >
  void State<OperatingPoint, priority_type, error_coef_type>::dump(const std::string& prefix) const
  {
    // print the global information
    std::cout << prefix << " Number of constraints: " << constraints.size() << std::endl;
    const std::string problem =  problem_is_changed ? "YES!" : "NO!";
    std::cout << prefix << " We need to force the finding of a new solution? " << problem << std::endl;
    std::cout << prefix << " (Without considering goals and runtime information)" << std::endl;
    std::cout << prefix << std::endl;
    std::cout << prefix << " ----------------------------------------------------------" << std::endl;
    std::cout << prefix << " -- Last known best Operating Point" << std::endl;
    std::cout << prefix << " ----------------------------------------------------------" << std::endl;

    if (best_operating_point_found)
    {
      print_whole_op<OperatingPoint>(best_operating_point_found, prefix);
    }
    else
    {
      std::cout << prefix << " We haven't found any Operating Points yet" << std::endl;
    }

    std::cout << prefix << std::endl;
    std::cout << prefix << " ----------------------------------------------------------" << std::endl;
    std::cout << prefix << " -- Optimization problem representation" << std::endl;
    std::cout << prefix << " ----------------------------------------------------------" << std::endl;
    std::cout << prefix << std::endl;
    std::cout << prefix << " The optimization problem is represented as a filtering of" << std::endl;
    std::cout << prefix << " the Operating Points, from the one(s) invalidated by the" << std::endl;
    std::cout << prefix << " top priority constraint, to the valid Operating Points" << std::endl;
    std::cout << prefix << std::endl;


    // print information about the constraints
    for ( const auto& constraint_pair : constraints )
    {
      std::cout << prefix << " ---- Constraint with priority " << constraint_pair.first << std::endl;
      std::cout << prefix << " ----------------------------------------------------------" << std::endl;
      std::cout << prefix << std::endl;
      constraint_pair.second->dump(prefix);
      std::cout << prefix << std::endl;
    }


    // print information about the rank
    std::cout << prefix << " ---- Valid Operating Points " << std::endl;
    std::cout << prefix << " ----------------------------------------------------------" << std::endl;
    std::cout << prefix << std::endl;
    rank->dump(prefix);
  }

}

#endif // MARGOT_STATE_HDR
