/* core/constraint.hpp
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

  /**
   * @brief Interface of a generic Constraint
   *
   * @tparam OperatingPoint The type which defines the Operating Point characteristics
   * @tparam error_coef_type The type used to compute the error coefficient of the application knowledge
   *
   * @details
   * This class aims at representing a constraint of the constrained multi-objective optimization
   * problem solved by a state.
   * This interface hides all the details of the constraint, providing a common interface for the
   * state to handle the Operating Points.
   *
   * A constraint act as a filter, storing all the Operating Points that are not valid for the
   * current contraint, but are valid for the higher priority constraints.
   * To efficiently adapt to changes on the goal value or to react promptly to changes in the execution
   * environment, a constraint exploit a View of the Operating Point, which sort the application
   * knowledge according to target field of the constraint.
   */
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
      using OperatingPointPtr = typename Knowledge<OperatingPoint>::OperatingPointPtr;


      /**
       * @brief Definition of the stream of Operating Point
       *
       * @details
       * This container is used as interface from the other component to find
       * the best OperatingPoint from the container
       */
      using OPStream = typename Knowledge<OperatingPoint>::OPStream;




      /******************************************************************
       *  METHODS TO UPDATE THE APPLICATION KNOWLEDGE
       ******************************************************************/


      /**
       * @brief Add the target Operating Point to the View of the constraint
       *
       * @param [in] new_op A shared pointer which represent the new Operating Point
       *
       * @details
       * This method change only the view of the constraint, it has no effect on
       * the Operating Point blocked by the constraint.
       * This method is meant to be used when the application add a new Operating
       * Point in the AS-RTM.
       */
      virtual void add( const OperatingPointPtr& new_op ) = 0;


      /**
       * @brief Remove the target Operating Point from the constraint
       *
       * @param [in] op A shared pointer to the Operating Point to remove
       *
       * @details
       * This method removes the target Operating Point from the view and from
       * the Operating Point blocked by this constraint (if present).
       * This method is meant to be used when the application remove an Operating
       * Point from the AS-RTM.
       */
      virtual void remove(const OperatingPointPtr& op ) = 0;


      /**
       * @brief Set the Operating Points in the view of the Constraint
       *
       * @param [in] application_knowledge The application knowledge
       *
       * @details
       * This method removes all the current Operating Point of the view (if any)
       * and insert all the Operating Point of the knowledge base.
       * This method do not affect the Operating Point blocked by the constraint.
       * This method is meant to after the creation of a Constraint, to initialize
       * its view.
       */
      virtual void set( const Knowledge<OperatingPoint>& application_knowledge ) = 0;




      /******************************************************************
       *  METHODS TO SET THE RUNTIME INFORMATION
       ******************************************************************/


      /**
       * @brief Update the runtime information provider of the constraint
       *
       * @param [in] runtime_information The knowledge runtime information provider
       *
       * @details
       * This method is meant to be used by the AS-RTM every time the application
       * add a new information provider or removes all the information providers.
       */
      virtual void set_field_adaptor( const KnowledgeAdaptor< OperatingPoint, error_coef_type >& runtime_information ) = 0;




      /******************************************************************
       *  METHODS TO EVALUATE A STREAM OF OPERATING POINTS
       ******************************************************************/


      /**
       * @brief Get the closest Operating Points to the constraint goal
       *
       * @return A OPStream with all the closest Operating Points (if any)
       *
       * @details
       * This method returns an OPStream with all the Operating Point blocked
       * by this contraint, which are the closest to achive the goal of this
       * constraints.
       */
      virtual OPStream get_closest( void ) const = 0;


      /**
       * @brief Prune the input stream of Operating Points, according to the constraint's goal
       *
       * @param [in] ops The OPStream of the Operating Points to evaluate
       *
       * @return An OPStream pruned of the sub-optimal Operating Points
       *
       * @details
       * A constraints express a binary decision: either an Operating Point achieve the
       * constraint's goal or it doesn't.
       * This method aims at pruning the input stream of Operating Point, removing all
       * the sub-optimal Operating Point.
       * In particular, if there is at least one Operating Point, in the input stream,
       * that achieves the constraint's goal, then this method removes all the Operating
       * Points that don't achieve the constraint's goal. Otherwise, if no Operating
       * Point of the input stream achieves the constraint's goal, this method removes
       * all the Operating Points but the ones that are closer to achive the constraint's goal.
       */
      virtual OPStream narrow( const OPStream& ops ) const = 0;


      /**
       * @brief Retrieves all the Operating Points blocked by the constraint
       *
       * @return An OPStream with all the Operating Points blocked by the constraint
       */
      virtual OPStream to_stream( void ) const = 0;


      /**
       * @brief Append to input stream, all the Operating Points blocked by the constraint
       *
       * @param [in,out] ops The OPStream of all the Operating Points blocked up to the current constraint
       *
       * @details
       * This method takes as input an OPStream which contains all the Operting Point blocked
       * by lower priority constraints and append to it, all the Operating Points blocked
       * by the current constraint.
       */
      virtual void append_to( OPStream& ops ) const = 0;


      /**
       * @brief Removes from the input stream, all the Operating Points blocked by this constraint
       *
       * @param [in] input The OPStream of the Operating Points to evaluate
       * @param [out] output The input OPStream pruned of the blocked Operating Points
       *
       * @details
       * For each Operating Point in the input stream, this method checks if we are blocking
       * them. If the latter statement is false, we store them in the output stream.
       * In other word, this method removes from the input stream, all the Operating Points
       * blocked by the current constraint.
       *
       * @note
       * The output stream is cleared before evaluating the input stream.
       */
      virtual void remove_blocked_ops_from( OPStream& input, OPStream& output ) const = 0;




      /******************************************************************
       *  METHODS TO MANAGE THE BLOCKED OPS
       ******************************************************************/


      /**
       * @brief Removes all the Operating Points, blocked by this constraint
       *
       * @details
       * This method clear the internal container used to store all the blocked
       * Operating Points by this constraint.
       */
      virtual void clear( void ) = 0;


      /**
       * @brief Evaluate the input stream to block the Operating Points
       *
       * @param [in] input The OPStream to evaluate
       * @param [out] blocked All the Operating Points blocked by the constraint
       *
       * @details
       * This method evaluates all the Operating Points in the input stream and
       * stores to the blocked stream all the Operating Points that don't achieve
       * this constraint's goal. It does the opposite of filter_add.
       * All the blocked Operating Points are also stored in the internal container
       * that keep tracks of the blocked Operating Points.
       */
      virtual void filter_initialize( OPStream& input, OPStream& blocked ) = 0;


      /**
       * @brief Evaluate the input stream to block the Operating Points
       *
       * @param [in] input The OPStream to evaluate
       * @param [out] output All the Operating Points not blocked by the constraint
       *
       * @details
       * This method evaluates all the Operating Points in the input stream and
       * stores to the output stream all the Operating Points that achieves
       * this constraint's goal. It does the opposite of filter_initialize.
       * All the blocked Operating Points are also stored in the internal container
       * that keep tracks of the blocked Operating Points.
       */
      virtual void filter_add( OPStream& input, OPStream& output ) = 0;


      /**
       * @brief Evaluate the input stream to unblock the Operating Points
       *
       * @param [in] input The OPStream to evaluate
       * @param [out] output All the Operating Points not blocked by the constraint
       *
       * @details
       * For each Operating Point in the input stream, this constraint checks if
       * it is blocking them, i.e. if the Operating Point is in the internal container
       * that keeps track of the blocked Operating Points.
       * If the latter statement is true, then this method remove the target Operting Point
       * from the internal container of the blocked ones. Otherwise, store the Operting
       * Point in the output OPstream.
       */
      virtual void filter_remove( OPStream& input, OPStream& output ) = 0;


      /**
       * @brief The method used to dynamically react to changes.
       *
       * @param [out] invalidated_ops An OPStream with all the Operating Points that are no more valid
       * @param [in] validated_ops An OPStream with all the Operating Points that are become valid
       *
       * @details
       * This method takes into account changes in the goal value and changes from the field adaptor,
       * observed by the related monitor (if any).
       * If the situation is going worst, the method stores in the invalidated_ops stream all the
       * Operating Point that doesn't achive the constraint's goal anymore, therefore they should be
       * blocked by the constraint (if they are valid for the higher priority constraint).
       * If the situation is going better, this method removes from the internal container of the
       * blocked Operating Points the ones that are become valid and store them to the validated_ops
       * stream.
       */
      virtual void update( OPStream& invalidated_ops, OPStream& validated_ops ) = 0;


      /**
       * @brief Virtual decustructor of the interface
       */
      virtual ~ConstraintHandler( void )
      {}

  };


  namespace helper
  {


    /**
     * @brief helper struct, to retrieve the correct bound value, from the comparison function
     *
     * @tparam cf The enumerator that represents the target comparison function
     *
     * @details
     * A constraint represents a property that the most suitable configuration must achieve.
     * It involves a comparison function (e.g. greater than or less than), a numerical value
     * and a confidence, expressed in terms of how many time the standard deviation is kept
     * into account.
     * This struct provides a global interface that selects automatically the correct bound
     * for evaluating an Operating Point, given the target comparison function. For example,
     * if the comparison function is greater than, then the constraint must use a lower bound.
     * This struct takes advantage of partial specialization to select the correct bound.
     * Since this struct represents the general case, you should never be able to use this
     * struct.
     */
    template< ComparisonFunctions cf >
    struct constraint;


    /**
     * @brief Specialization of the helper struct, for the "grater" comparison function
     *
     * @see helper::constraint
     */
    template< >
    struct constraint< ComparisonFunctions::GREATER >
    {

      /**
       * @brief Retrieves the correct bound for the "greater" comparison function
       *
       * @return The enumerator BoundType::LOWER as constexpr value
       */
      inline static constexpr BoundType get_bound( void )
      {
        return BoundType::LOWER;
      }
    };


    /**
     * @brief Specialization of the helper struct, for the "grater or equal" comparison function
     *
     * @see helper::constraint
     */
    template< >
    struct constraint< ComparisonFunctions::GREATER_OR_EQUAL >
    {

      /**
       * @brief Retrieves the correct bound for the "grater or equal" comparison function
       *
       * @return The enumerator BoundType::LOWER as constexpr value
       */
      inline static constexpr BoundType get_bound( void )
      {
        return BoundType::LOWER;
      }
    };


    /**
     * @brief Specialization of the helper struct, for the "less" comparison function
     *
     * @see helper::constraint
     */
    template< >
    struct constraint< ComparisonFunctions::LESS >
    {

      /**
       * @brief Retrieves the correct bound for the "less" comparison function
       *
       * @return The enumerator BoundType::UPPER as constexpr value
       */
      inline static constexpr BoundType get_bound( void )
      {
        return BoundType::UPPER;
      }
    };


    /**
     * @brief Specialization of the helper struct, for the "less or equal" comparison function
     *
     * @see helper::constraint
     */
    template< >
    struct constraint< ComparisonFunctions::LESS_OR_EQUAL >
    {

      /**
       * @brief Retrieves the correct bound for the "less or equal" comparison function
       *
       * @return The enumerator BoundType::UPPER as constexpr value
       */
      inline static constexpr BoundType get_bound( void )
      {
        return BoundType::UPPER;
      }
    };

  } // namespace helper



  /**
   * @brief Implementation of the ConstraintHandler interface
   *
   * @tparam OperatingPoint The type which defines the Operating Point characteristics
   * @tparam segment The target segment of the Operating Point
   * @tparam field_index The index of the target field within the target segment of the Operating Point
   * @tparam sigma Number of times the standard deviation of the field must kept into account
   * @tparam ConstraintGoal The type of the target goal, related to the constraint
   * @tparam error_coef_type The type used to compute the error coefficient of the application knowledge
   *
   * @see ConstraintHandler
   *
   * @details
   * This class is an implementation of a generic constraint, represented by the interface ConstraintHandler.
   * While the interface expose toward the framework all the methods to manage Operating Point, this class
   * actually uses all the information required to evaluate the constraint.
   */
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
      using OperatingPointPtr = typename ConstraintHandler<OperatingPoint>::OperatingPointPtr;


      /**
       * @brief Definition of the stream of Operating Point
       *
       * @details
       * This container is used as interface from the other component to find
       * the best OperatingPoint from the container
       */
      using OPStream = typename ConstraintHandler<OperatingPoint>::OPStream;


      /**
       * @brief Definition of the field related to this constraint
       */
      using ConstraintField = OPField< segment, helper::constraint<ConstraintGoal::comparison_function>::get_bound(), field_index, sigma, int >;


      /**
       * @brief Definition of the view on the Operating Point, used by this constraint
       */
      using MyView = View< OperatingPoint, FieldComposer::SIMPLE, ConstraintField >;


      /**
       * @brief The container used to store the Operating Points blocked by this constraint
       */
      using Container = std::unordered_set< OperatingPointPtr >;


      /**
       * @brief Pointer of the runtime information provider for the target field of the Operating Point
       */
      using MyFieldAdaptor = typename KnowledgeAdaptor< OperatingPoint, error_coef_type >::FieldAdaptorPtr;


      /**
       * @brief Aliasing of the type of the values stored in the view
       */
      using view_type = typename MyView::value_type;


      /**
       * @brief Aliasing of the type of the goal value
       */
      using goal_value_type = typename ConstraintGoal::value_type;


      /**
       * @brief The actual type of the goal value, once it is been adjusted by runtime information
       */
      using value_type = decltype( error_coef_type{} * goal_value_type{});


      /**
       * @brief Default construcor which initialize the goal
       *
       * @param [in] goal_value The Goal which represents the constraint
       *
       * @details
       * The constructor makes a copy of the target Goal. Every time the user will change the goal
       * value, the constraint automatically reacts to that change.
       * The trivial constructor in not available, because for implementation reason there must
       * always be a Goal which might validate an Operating Point.
       */
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


      /**
       * @brief Implementation of the ConstraintHandler "add" method
       *
       * @param [in] new_op A shared pointer which represent the new Operating Point
       *
       * @see ConstraintHandler
       */
      void add( const OperatingPointPtr& new_op )
      {
        knowledge_view.add(new_op);
      }


      /**
       * @brief Implementation of the ConstraintHandler "remove" method
       *
       * @param [in] op  A shared pointer to the Operating Point to remove
       *
       * @see ConstraintHandler
       */
      void remove(const OperatingPointPtr& op )
      {
        knowledge_view.remove( op );
        blocked_ops.erase(op);
      }


      /**
       * @brief Implementation of the ConstraintHandler "set" method
       *
       * @param [in] application_knowledge The application knowledge
       *
       * @see ConstraintHandler
       */
      void set( const Knowledge<OperatingPoint>& application_knowledge )
      {
        knowledge_view.clear();
        knowledge_view.add(application_knowledge.begin(), application_knowledge.end());
      }




      /******************************************************************
       *  METHODS TO SET THE RUNTIME INFORMATION
       ******************************************************************/


      /**
       * @brief Implementation of the ConstraintHandler "set_field_adaptor" method
       *
       * @param [in] runtime_information The knowledge runtime information provider
       *
       * @see ConstraintHandler
       */
      void set_field_adaptor( const KnowledgeAdaptor< OperatingPoint, error_coef_type >& runtime_information )
      {
        knowledge_adaptor = runtime_information.template get_field_adaptor<segment, field_index>();
      }




      /******************************************************************
       *  METHODS TO EVALUATE A STREAM OF OPERATING POINTS
       ******************************************************************/


      /**
       * @brief Implementation of the ConstraintHandler "get_closest" method
       *
       * @return A OPStream with all the closest Operating Points (if any)
       *
       * @see ConstraintHandler
       */
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


      /**
       * @brief Implementation of the ConstraintHandler "narrow" method
       *
       * @param [in] ops The OPStream of the Operating Points to evaluate
       *
       * @return An OPStream pruned of the sub-optimal Operating Points
       *
       * @see ConstraintHandler
       */
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


      /**
       * @brief Implementation of the ConstraintHandler "to_stream" method
       *
       * @return An OPStream with all the Operating Points blocked by the constraint
       *
       * @see ConstraintHandler
       */
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


      /**
       * @brief Implementation of the ConstraintHandler "append_to" method
       *
       * @param [in,out] ops The OPStream of all the Operating Points blocked up to the current constraint
       *
       * @see ConstraintHandler
       */
      void append_to( OPStream& ops ) const
      {
        for ( const auto& op : blocked_ops )
        {
          ops.emplace_back(op);
        }
      }


      /**
       * @brief Implementation of the ConstraintHandler "remove_blocked_ops_from" method
       *
       * @param [in] input The OPStream of the Operating Points to evaluate
       * @param [out] output The input OPStream pruned of the blocked Operating Points
       *
       * @see ConstraintHandler
       */
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


      /**
       * @brief Implementation of the ConstraintHandler "clear" method
       *
       * @see ConstraintHandler
       */
      void clear( void )
      {
        blocked_ops.clear();
      }


      /**
       * @brief Implementation of the ConstraintHandler "filter_initialize" method
       *
       * @param [in] input The OPStream to evaluate
       * @param [out] blocked All the Operating Points blocked by the constraint
       *
       * @see ConstraintHandler
       */
      void filter_initialize( OPStream& input, OPStream& blocked )
      {
        // make sure that the output stream is empty
        blocked.clear();

        // block all the invalid Operating Points;
        for ( auto && op : input )
        {
          if ( !target_goal.template check<view_type, value_type >(knowledge_view.evaluate_op(op), last_check_value) )
          {
            blocked_ops.emplace(op);
            blocked.emplace_back(op);
          }
        }
      }


      /**
       * @brief Implementation of the ConstraintHandler "filter_add" method
       *
       * @param [in] input The OPStream to evaluate
       * @param [out] output All the Operating Points not blocked by the constraint
       *
       * @see ConstraintHandler
       */
      void filter_add( OPStream& input, OPStream& output )
      {
        // make sure that the output stream is empty
        output.clear();

        // block all the invalid Operating Points;
        for ( auto && op : input )
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


      /**
       * @brief Implementation of the ConstraintHandler "filter_remove" method
       *
       * @param [in] input The OPStream to evaluate
       * @param [out] output All the Operating Points not blocked by the constraint
       *
       * @see ConstraintHandler
       */
      void filter_remove( OPStream& input, OPStream& output )
      {
        // make sure that the output stream is empty
        output.clear();

        // remove all the input Operating Points
        for ( auto && op : input )
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


      /**
       * @brief Implementation of the ConstraintHandler "update" method
       *
       * @param [out] invalidated_ops An OPStream with all the Operating Points that are no more valid
       * @param [in] validated_ops An OPStream with all the Operating Points that are become valid
       *
       * @see ConstraintHandler
       */
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


      /**
       * @brief The reference goal for the constraint
       */
      ConstraintGoal target_goal;


      /**
       * @brief The last value used to update the constraint
       *
       * @details
       * This value takes into account the information provided by the runtime
       * information provider.
       */
      value_type last_check_value;


      /**
       * @brief The sorted view over the application knowledge
       */
      MyView knowledge_view;


      /**
       * @brief All the Operating Point blocked by this constraint
       */
      Container blocked_ops;


      /**
       * @brief The pointer to the runtime information provider, related to the target field
       */
      MyFieldAdaptor knowledge_adaptor;

  };

}

#endif // MARGOT_CONSTRAINT_HDR
