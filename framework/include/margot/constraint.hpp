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

#include "margot/operating_point.hpp"
#include "margot/field_adaptor.hpp"
#include "margot/knowledge_adaptor.hpp"
#include "margot/view.hpp"
#include "margot/knowledge_base.hpp"

namespace margot
{

  template< ComparisonFunction cf >
  struct constraint_helper;

  template< >
  struct constraint_helper< ComparisonFunction::GREATER >
  {
    inline static constexpr BoundType get_bound( void )
    {
      return BoundType::LOWER;
    }

    template< class T >
    inline static bool compare( const T value, const T goal )
    {
      return value > goal;
    }
  };

  template< >
  struct constraint_helper< ComparisonFunction::GREATER_OR_EQUAL >
  {
    inline static constexpr BoundType get_bound( void )
    {
      return BoundType::LOWER;
    }

    template< class T >
    inline static bool compare( const T value, const T goal )
    {
      return value >= goal;
    }
  };

  template< >
  struct constraint_helper< ComparisonFunction::LESS >
  {
    inline static constexpr BoundType get_bound( void )
    {
      return BoundType::UPPER;
    }

    template< class T >
    inline static bool compare( const T value, const T goal )
    {
      return value < goal;
    }
  };

  template< >
  struct constraint_helper< ComparisonFunction::LESS_OR_EQUAL >
  {
    inline static constexpr BoundType get_bound( void )
    {
      return BoundType::UPPER;
    }

    template< class T >
    inline static bool compare( const T value, const T goal )
    {
      return value <= goal;
    }
  };


  template< class OperatingPoint,
            OperatingPointSegments segment,
            std::size_t field_index,
            int sigma,
            ComparisonFunction cf,
            typename error_coef_type = float >
  class Constraint
  {

      // statically check the template argument
      static_assert(traits::is_operating_point<OperatingPoint>::value,
                    "Error: the Constraint handles object with is_operating_point trait");

    public:

      /**
       * @brief Explicit definition to an Operating Point pointer
       */
      using OperatingPointPtr = std::shared_ptr<OperatingPoint>;


      /**
       * @brief Definition of the stream of Operating Point
       *
       * @details
       * This container is used as interface from the other component to find
       * the best OperatingPoint from the container
       */
      using OPStream = std::deque< OperatingPointPtr >;

      using ConstraintField = OPField< segment, constraint_helper<cf>::get_bound(), field_index, sigma, int >;
      using MyView = View<OperatingPoint, FieldComposer::SIMPLE, ConstraintField >;

      using value_type = typename MyView::value_type;

      using Container = std::unordered_set< OperatingPointPtr >;

      using MyFieldAdaptor = typename KnowledgeAdaptor< OperatingPoint, error_coef_type >::FieldAdaptorPtr;



      Constraint( const Knowledge< OperatingPoint >& application_knowledge,
                  const KnowledgeAdaptor< OperatingPoint, error_coef_type >& runtime_information,
                  const value_type goal_value )
      {
        // populate the view (the number in the constructor is meaningless for the constraint ), if any
        knowledge_view = MyView(1);

        // set the goal values
        current_goal_value = goal_value;
        last_check_value = goal_value;
      }


      /******************************************************************
       *  METHODS TO UPDATE THE VIEW ON THE KNOWLEDGE
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
       *  METHODS TO MANAGE THE BLOCKED OPS
       ******************************************************************/

      void clear( void )
      {
        blocked_ops.clear();
      }


      void filter_add( OPStream& input, OPStream& output )
      {
        // make sure that the output stream is empty
        output.clear();

        // block all the invalid Operating Points;
        for( auto&& op : input )
        {
          if ( constraint_helper<cf>::compare(knowledge_view.evaluate_op(op), last_check_value) )
            output.emplace(op);
          else
            blocked_ops.emplace(op);
        }
      }


      void filter_remove( OPStream& input, OPStream& output )
      {
        // make sure that the output stream is empty
        output.clear();

        // remove all the input Operating Points
        for( auto&& op : input )
        {
          const auto elem_it = blocked_ops.find(op);
          if (elem_it != blocked_ops.end())
            blocked_ops.erase(elem_it);
          else
            output.emplace(op);
        }
      }


      void update( OPStream& invalidated_ops, OPStream& validated_ops )
      {
        // make sure that the output streams are empty
        invalidated_ops.clear();
        validated_ops.clear();

        // get the error coefficient
        const error_coef_type coef = knowledge_adaptor ? knowledge_adaptor->get_error_coefficient() : 1.0f;

        // compute the final goal value
        const value_type final_goal_value = current_goal_value * static_cast< value_type >(coef);

        // get the op stream of the difference
        auto diff_stream = knowledge_view.range(final_goal_value, last_check_value);

        // check if it is going worst
        if (constraint_helper<cf>::compare(final_goal_value, last_check_value))
        {
          // we need to invalidate the ops in the range, but we don't know if they
          // are blocked by an upper constraint, so we can only signal that we
          // we are going to invalidate the Operating Points.
          diff_stream.swap(invalidated_ops);
        }
        else // it is going better
        {
          // we remove the diff ops from the container
          for( const auto& op : blocked_ops)
          {
            // check if we are the one that block it
            const auto elem_it = knowledge_view.find(op);
            if (elem_it != blocked_ops.end())
            {
              // then we must release it
              validated_ops.emplace(op);
              blocked_ops.erase(elem_it);
            }
            // if we are not blocking them, then we don't have to do nothing
            // because it means that some upper constraint is blocking it
          }
        }

        // finally, we can update the last check goal value
        last_check_value = final_goal_value;
      }




    private:

      value_type current_goal_value;
      value_type last_check_value;


      MyView knowledge_view;

      Container blocked_ops;

      MyFieldAdaptor knowledge_adaptor;


  };



}

#endif // MARGOT_CONSTRAINT_HDR
