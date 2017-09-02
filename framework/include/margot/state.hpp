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


#ifndef MARGOT_STATE_HPP
#define MARGOT_STATE_HPP

#include <cstddef>
#include <memory>
#include <map>
#include <vector>
#include <cassert>

#include "margot/operating_point.hpp"
#include "margot/knowledge_adaptor.hpp"
#include "margot/knowledge_base.hpp"
#include "margot/goal.hpp"
#include "margot/enums.hpp"
#include "margot/constraint.hpp"
#include "margot/rank.hpp"

namespace margot
{

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
      using OperatingPointPtr = std::shared_ptr< OperatingPoint >;


      /**
       * @brief Definition of the stream of Operating Point
       */
      using OPStream = std::vector< OperatingPointPtr >;


      using ConstraintPtr = std::shared_ptr< ConstraintHandler< OperatingPoint, error_coef_type > >;


      using RankPtr = std::shared_ptr< RankInterface< OperatingPoint > >;


      using ConstraintStack = std::map< priority_type, ConstraintPtr >;


      State( void ): problem_is_changed(true)
      {
        // we are just interested in one field that, for sure, is available: the first software knob
        using dummy_field = OPField< OperatingPointSegments::SOFTWARE_KNOBS, BoundType::LOWER, 0, 0>;

        // create a dummy rank, just to have a definition of best
        // and to have a container for all the Operating Points
        rank.reset( new Rank< OperatingPoint, RankObjective::MINIMIZE, FieldComposer::SIMPLE, dummy_field >( 1.0f ) );
      }


      /******************************************************************
       *  CONSTRAINTS MANAGEMENT
       ******************************************************************/


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

      void add_operating_points( OPStream& new_ops )
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

        for ( auto& constraint_pair : constraints )
        {
          ops_to_add.swap(new_ops);
          constraint_pair.second->filter_add( ops_to_add, new_ops );
        }

        // add them to the rank
        rank->add(new_ops);

        // we have to solve again the optimization problem
        problem_is_changed = true;
      }


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
       *  METHODS TO ACTUALLY SOLVE THE OPTIMIZATION PROBLEM
       ******************************************************************/

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


      OperatingPointPtr get_best_operating_point( void )
      {
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



    private:

      OperatingPointPtr get_best_from_stream( OPStream& input_ops, typename ConstraintStack::iterator& constraint_it ) const
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

      ConstraintStack constraints;

      bool problem_is_changed;
      OperatingPointPtr best_operating_point_found;

      RankPtr rank;


  };

}

#endif // MARGOT_STATE_HPP
