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

#ifndef MARGOT_CONSTRAINT_HDR
#define MARGOT_CONSTRAINT_HDR

#include <cstddef>
#include <memory>
#include <unordered_set>
#include <vector>

#include "margot/operating_point.hpp"
#include "margot/field_adaptor.hpp"
#include "margot/knowledge_adaptor.hpp"
#include "margot/view.hpp"
#include "margot/knowledge_base.hpp"
#include "margot/goal.hpp"
#include "margot/enums.hpp"

namespace margot
{


  template< class OperatingPoint, typename error_coef_type = float >
  class ConstraintHandler
  {
      // statically check the template argument
      static_assert(traits::is_operating_point<OperatingPoint>::value,
                    "Error: the Constraint handles object with is_operating_point trait");

    public:

      /**
       * @brief Explicit definition to an Operating Point pointer
       */
      using OperatingPointPtr = std::shared_ptr< OperatingPoint >;


      /**
       * @brief Definition of the stream of Operating Point
       */
      using OPStream = std::vector< OperatingPointPtr >;


      /******************************************************************
       *  METHODS TO UPDATE THE APPLICATION KNOWLEDGE
       ******************************************************************/


      virtual void add( const OperatingPointPtr& new_op ) = 0;

      virtual void remove(const OperatingPointPtr& op ) = 0;

      virtual void set( const Knowledge<OperatingPoint>& application_knowledge ) = 0;

      /******************************************************************
       *  METHODS TO SET THE RUNTIME INFORMATION
       ******************************************************************/

      virtual void set_field_adaptor( const KnowledgeAdaptor< OperatingPoint, error_coef_type >& runtime_information ) = 0;


      /******************************************************************
       *  METHODS TO EVALUATE A STREAM OF OPERATING POINTS
       ******************************************************************/

      virtual OPStream get_closest( void ) const = 0;

      virtual OPStream narrow( const OPStream& ops ) const = 0;

      virtual OPStream to_stream( void ) const = 0;

      virtual void append_to( OPStream& ops ) const = 0;

      virtual void remove_blocked_ops_from( OPStream& input, OPStream& output ) const = 0;


      /******************************************************************
       *  METHODS TO MANAGE THE BLOCKED OPS
       ******************************************************************/

      virtual void clear( void ) = 0;


      virtual void filter_initialize( OPStream& input, OPStream& blocked ) = 0;


      virtual void filter_add( OPStream& input, OPStream& output ) = 0;


      virtual void filter_remove( OPStream& input, OPStream& output ) = 0;


      virtual void update( OPStream& invalidated_ops, OPStream& validated_ops ) = 0;

      virtual ~ConstraintHandler( void )
      {}

  };


  namespace helper
  {
    template< ComparisonFunctions cf >
    struct constraint;

    template< >
    struct constraint< ComparisonFunctions::GREATER >
    {
      inline static constexpr BoundType get_bound( void )
      {
        return BoundType::LOWER;
      }
    };

    template< >
    struct constraint< ComparisonFunctions::GREATER_OR_EQUAL >
    {
      inline static constexpr BoundType get_bound( void )
      {
        return BoundType::LOWER;
      }
    };

    template< >
    struct constraint< ComparisonFunctions::LESS >
    {
      inline static constexpr BoundType get_bound( void )
      {
        return BoundType::UPPER;
      }
    };

    template< >
    struct constraint< ComparisonFunctions::LESS_OR_EQUAL >
    {
      inline static constexpr BoundType get_bound( void )
      {
        return BoundType::UPPER;
      }
    };
  }



  template< class OperatingPoint,
            OperatingPointSegments segment,
            std::size_t field_index,
            int sigma,
            class ConstraintGoal,
            typename error_coef_type = float >
  class Constraint: public ConstraintHandler< OperatingPoint, error_coef_type >
  {

      // statically check the template argument
      static_assert(traits::is_operating_point<OperatingPoint>::value,
                    "Error: the Constraint handles object with is_operating_point trait");

    public:

      /**
       * @brief Explicit definition to an Operating Point pointer
       */
      using OperatingPointPtr = std::shared_ptr< OperatingPoint >;


      /**
       * @brief Definition of the stream of Operating Point
       */
      using OPStream = std::vector< OperatingPointPtr >;

      using ConstraintField = OPField< segment, helper::constraint<ConstraintGoal::comparison_function>::get_bound(), field_index, sigma, int >;
      using MyView = View< OperatingPoint, FieldComposer::SIMPLE, ConstraintField >;

      using Container = std::unordered_set< OperatingPointPtr >;

      using MyFieldAdaptor = typename KnowledgeAdaptor< OperatingPoint, error_coef_type >::FieldAdaptorPtr;


      using view_type = typename MyView::value_type;

      using goal_value_type = typename ConstraintGoal::value_type;

      using value_type = decltype( error_coef_type{} * goal_value_type{});



      Constraint( const ConstraintGoal& goal_value )
      {
        // populate the view (the number in the constructor is meaningless for the constraint ), if any
        knowledge_view = MyView(1);

        // set the goal values
        target_goal = goal_value;
        last_check_value = static_cast<value_type>(target_goal.get());
      }


      /******************************************************************
       *  METHODS TO UPDATE THE APPLICATION KNOWLEDGE
       ******************************************************************/


      void add( const OperatingPointPtr& new_op )
      {
        knowledge_view.add(new_op);
      }

      void remove(const OperatingPointPtr& op )
      {
        knowledge_view.remove( op );
        blocked_ops.erase(op);
      }


      void set( const Knowledge<OperatingPoint>& application_knowledge )
      {
        knowledge_view.clear();
        knowledge_view.add(application_knowledge.begin(), application_knowledge.end());
      }

      /******************************************************************
       *  METHODS TO SET THE RUNTIME INFORMATION
       ******************************************************************/

      void set_field_adaptor( const KnowledgeAdaptor< OperatingPoint, error_coef_type >& runtime_information )
      {
        knowledge_adaptor = runtime_information.template get_field_adaptor<segment, field_index>();
      }


      /******************************************************************
       *  METHODS TO EVALUATE A STREAM OF OPERATING POINTS
       ******************************************************************/

      OPStream get_closest( void ) const
      {
        // if this constraint do not block ant op, we shall return an empty stream
        if (blocked_ops.empty())
        {
          return OPStream{};
        }

        // otherwise we must search for the best ones
        OPStream closest_ops;

        // assuming that the first OP blocked by this
        // constraint is the closest one
        auto op_it = blocked_ops.cbegin();
        view_type closest_op_value = knowledge_view.evaluate_op(*op_it);
        closest_ops.emplace_back(*op_it);

        // loop over the other OPs and make sure to get the closest ones
        const auto end_it = blocked_ops.cend();

        for ( ++op_it ; op_it != end_it; ++op_it )
        {
          // get the value of the Operating Point
          const view_type op_value = knowledge_view.evaluate_op(*op_it);

          if ( op_value == closest_op_value )
          {
            // if they have the same value, it's one of the closest ones
            closest_ops.emplace_back(*op_it);
          }
          else if ( target_goal.template check< view_type, view_type>(op_value, closest_op_value) )
          {
            // if the current op has a different and better value,
            // we have found a whole new level of closest
            closest_ops.clear();
            closest_ops.emplace_back(*op_it);
            closest_op_value = op_value;
          }
        }

        // these are the closest ops
        return closest_ops;
      }

      OPStream narrow( const OPStream& ops ) const
      {
        // if the stream is empty, we shall return an empty stream
        if (ops.empty())
        {
          return OPStream{};
        }

        // otherwise we must search for the best ones
        OPStream best_ops;

        // assuming that the first OP of the container is the one of the best ones
        auto op_it = ops.cbegin();
        view_type best_op_value = knowledge_view.evaluate_op(*op_it);
        bool best_op_is_valid = target_goal.template check< view_type, value_type >(best_op_value, last_check_value);
        best_ops.emplace_back(*op_it);

        // loop over the other OPs and make sure to get the best ones
        const auto end_it = ops.cend();

        for ( ++op_it ; op_it != end_it; ++op_it )
        {
          // get the value of the evaluated Operating Point
          const view_type op_value = knowledge_view.evaluate_op(*op_it);

          // check if it is valid
          const bool is_valid = target_goal.template check< view_type, value_type >(op_value, last_check_value);

          // if it is valid, we are ok with it
          if (is_valid)
          {
            // if the previous best was not valid, we must discard the previous ones
            if (!best_op_is_valid)
            {
              best_ops.clear();
              best_op_is_valid = true;
            }

            best_ops.emplace_back(*op_it);
          }
          else if ( !best_op_is_valid && (op_value == best_op_value) )
          {
            // if this op have the same value of the best one and the best one
            // is not valid, then this op it's one of the best
            best_ops.emplace_back(*op_it);
          }
          else if ( !best_op_is_valid
                    && target_goal.template check< view_type, view_type >(op_value, best_op_value) )
          {
            // if this op have a better value of the best one and the best one is not valid,
            // then we have found a whole new level of best
            best_ops.clear();
            best_ops.emplace_back(*op_it);
            best_op_value = op_value;
          }
        }

        // these are the best ops
        return best_ops;
      }

      OPStream to_stream( void ) const
      {
        // if we don't block any Operating Point
        if (blocked_ops.empty())
        {
          // return an empty stream
          return OPStream{};
        }

        // otherwise build the stream fro the blocked Operating Points
        OPStream result;
        result.reserve(blocked_ops.size());

        for ( const auto& op : blocked_ops )
        {
          result.emplace_back(op);
        }

        return result;
      }


      void append_to( OPStream& ops ) const
      {
        for ( const auto& op : blocked_ops )
        {
          ops.emplace_back(op);
        }
      }


      void remove_blocked_ops_from( OPStream& input, OPStream& output ) const
      {
        // make sure that the output stream is empty
        output.clear();

        // prune from the input the blocked Operating Points
        const auto end_it = blocked_ops.cend();

        for ( const auto& op : input )
        {
          // check if we are blocking the Operating Point
          const auto op_it = blocked_ops.find(op);

          if ( op_it == end_it )
          {
            // store them in the output stream
            output.emplace_back(op);
          }
        }
      }


      /******************************************************************
       *  METHODS TO MANAGE THE BLOCKED OPS
       ******************************************************************/

      void clear( void )
      {
        blocked_ops.clear();
      }


      void filter_initialize( OPStream& input, OPStream& blocked )
      {
        // make sure that the output stream is empty
        blocked.clear();

        // block all the invalid Operating Points;
        for ( auto&& op : input )
        {
          if ( !target_goal.template check<view_type, value_type >(knowledge_view.evaluate_op(op), last_check_value) )
          {
            blocked_ops.emplace(op);
            blocked.emplace_back(op);
          }
        }
      }


      void filter_add( OPStream& input, OPStream& output )
      {
        // make sure that the output stream is empty
        output.clear();

        // block all the invalid Operating Points;
        for ( auto&& op : input )
        {
          if ( target_goal.template check<view_type, value_type >(knowledge_view.evaluate_op(op), last_check_value) )
          {
            output.emplace_back(op);
          }
          else
          {
            blocked_ops.emplace(op);
          }
        }
      }


      void filter_remove( OPStream& input, OPStream& output )
      {
        // make sure that the output stream is empty
        output.clear();

        // remove all the input Operating Points
        for ( auto&& op : input )
        {
          const auto elem_it = blocked_ops.find(op);

          if (elem_it != blocked_ops.end())
          {
            blocked_ops.erase(elem_it);
          }
          else
          {
            output.emplace_back(op);
          }
        }
      }


      void update( OPStream& invalidated_ops, OPStream& validated_ops )
      {
        // make sure that the output streams are empty
        invalidated_ops.clear();
        validated_ops.clear();

        // get the error coefficient
        const error_coef_type coef = knowledge_adaptor ? knowledge_adaptor->get_error_coefficient()
                                     : static_cast<error_coef_type>(1.0f);

        // compute the final goal value
        const value_type final_goal_value = target_goal.get() * coef;

        // if they have the same value, quit, nothing is changed
        if ( final_goal_value == last_check_value )
        {
          return;
        }

        // otherwise, get the Operating Point involved in the change
        auto diff_stream = knowledge_view.range( last_check_value, final_goal_value );

        // otherwise, check if it is going worst
        if (target_goal.template check<value_type, value_type >(final_goal_value, last_check_value))
        {
          // we need to invalidate the ops in the range, but we don't know if they
          // are blocked by an upper constraint, so we can only signal that we
          // we are going to invalidate the Operating Points.
          // Moreover, we have to make sure to signal only the not valid ones.
          for ( const auto& op : diff_stream )
          {
            if ( !target_goal.template check<view_type, value_type >(knowledge_view.evaluate_op(op), final_goal_value) )
            {
              invalidated_ops.emplace_back(op);
            }
          }
        }
        else // it is going better
        {
          // we remove the diff ops from the blocked Operating Points
          for ( const auto& op : diff_stream)
          {
            // check if we are the one that block it
            const auto elem_it = blocked_ops.find(op);

            if (elem_it != blocked_ops.end())
            {
              // We are blocking the Operating Point, but we must be sure that the target Operating Point
              // is actually valid
              if (target_goal.template check<view_type, value_type >(knowledge_view.evaluate_op(op), final_goal_value))
              {
                // then we must release it
                validated_ops.emplace_back(op);
                blocked_ops.erase(elem_it);
              }
            }

            // if we are not blocking them, then we don't have to do nothing
            // because it means that some upper constraint is blocking it
          }
        }

        // finally, we can update the last check goal value
        last_check_value = final_goal_value;
      }




    private:

      ConstraintGoal target_goal;
      value_type last_check_value;


      MyView knowledge_view;

      Container blocked_ops;

      MyFieldAdaptor knowledge_adaptor;


  };



}

#endif // MARGOT_CONSTRAINT_HDR
