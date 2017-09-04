/* core/asrtm.hpp
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

#ifndef MARGOT_ASRTM_HDR
#define MARGOT_ASRTM_HDR

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <cstddef>
#include <cassert>
#include <mutex>
#include <functional>

#include "margot/operating_point.hpp"
#include "margot/knowledge_base.hpp"
#include "margot/knowledge_adaptor.hpp"
#include "margot/monitor.hpp"
#include "margot/state.hpp"


namespace margot
{

  /**
   * @brief The Application-Specific RunTime Manger implementation
   *
   * @tparam OperatingPoint The type which defines the Operating Point characteristics
   * @tparam state_id_type The type used to identify a state
   * @tparam priority_type The type used to represents the priority of a constraint
   * @tparam error_coef_type The type used to compute the error coefficient of the application knowledge
   *
   * @details
   * This class represent the interface of the mARGOt dynamic autotuner toward the appplication.
   * In particular it aims at:
   * 1) Handling the knowledge of the application (as an Operating Point list)
   * 2) Adapting the knowledge of the application according to the runtime information coming from the monitors
   * 3) Managing the constrained multi-objective optimization problems
   *
   * All the methods are mutex protected, to enforce a consistent internal state.
   *
   * @note
   * The state_id_type and priority_type types require that std::hash is able to compute their hash value.
   * It must be defined the < operator for priority type.
   * The error_coef_type must be a floating point aritmethic type.
   */
  template< class OperatingPoint, class state_id_type = std::string, typename priority_type = int, typename error_coef_type = float >
  class Asrtm
  {


      /**
       * @brief Internal state of the AS-RTM
       *
       * @details
       * To adapt according to runtime information coming from the monitor, we must be sure to
       * know which configuration the application is using. For this reason we use this small
       * state machine to keep track of the evolution of the application.
       */
      enum class ApplicationStatus
      {


        /**
         * @details
         * In this state, the application has retrieved a different configuration with respect the
         * previous one. However, the framework is not able to tell if the application is using
         * the new configuration or the previous one (it has an undefined behavior).
         */
        UNDEFINED,


        /**
         * @details
         * In this state, the application is using the best configuration selected by the framework.
         * This means, that we are able to take advantage of runtime information coming from the
         * application monitors.
         */
        TUNED
      };


      // statically check the template argument
      static_assert(traits::is_operating_point<OperatingPoint>::value,
                    "Error: the Application-Specific RunTime Manager handles object with is_operating_point trait");


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
       * @brief Container that stores all the optimization problems, i.e. the states
       *
       * @details
       * It uses an hash table to associate the id to the state. It requires fast loopkup, insert and
       * delete operations.
       */
      using StateMap = std::unordered_map< state_id_type, State< OperatingPoint, priority_type, error_coef_type > >;


      /**
       * @brief The definition of an iterator to the StateMap to identify the active state
       */
      using StateIt = typename StateMap::iterator;


      /**
       * @brief Explicit definition of the type of the configuration
       */
      using configuration_type = typename OperatingPoint::configuration_type;


      /**
       * @brief Definition of the type of the function used to clear a monitor's buffer
       */
      using clear_monitor_function_type = std::function<void(void)>;


      /**
       * @brief Definition of the container of all the functions that delete the monitor's buffer
       *
       * @details
       * This container requires a fast way to iterate through its elements.
       */
      using MonitorHandlers = std::vector< clear_monitor_function_type >;


      /**
       * @brief Default constructor of the AS-RTM
       */
      Asrtm( void ): status(ApplicationStatus::UNDEFINED)
      {
        current_optimizer = application_optimizers.end();
      }




      /******************************************************************
       *  METHODS TO MANAGE THE APPLICATION KNOWLEDGE
       ******************************************************************/


      /**
       * @brief Add a set of Operating Points to the AS-RTM
       *
       * @tparam T An object which implements the STL Container concept and stores Operating Points
       *
       * @param [in] op_list The container of the set of new Operating Points
       *
       * @return The number of Operating Points actually added to the AS-RTM
       *
       * @details
       * This methods insert new Operating Points in the knowledge base and in all the available
       * states in the AS-RTM.
       * Before adding an Operating Point to the knowledge base, it checks that does not exits
       * another Operating Point in the knowledge base with the same configuration.
       * Two configurations are the same if all the software knobs have the same mean value.
       * The returned number indicates how many Operating Points are really inserted in the
       * knowledge base.
       *
       * The container T must hold object with types OperatingPoint or OperatingPointPtr.
       * Any STL container can be used, however, the best practice is to use a container
       * which is fast to iterate, such as a std::vector< OperatingPoint >.
       */
      template< class T >
      std::size_t add_operating_points( const T& op_list )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);

        // this is meant to be the stream with the new Operating Points
        OPStream new_ops;

        // we assume that all the Operating Points in the list are really new
        new_ops.reserve(op_list.size());

        // loop over them and store the really new Operating Points
        for ( const auto& op : op_list )
        {
          // make sure to add a really new Operating Point
          const auto op_ptr = knowledge.add(op);

          if (op_ptr)
          {
            new_ops.emplace_back(op_ptr);
          }
        }

        // update also all the States that we have
        for ( auto& state_pair : application_optimizers )
        {
          state_pair.second.add_operating_points(new_ops);
        }

        return new_ops.size();
      }


      /**
       * @brief Remove a set of Operating Points from the AS-RTM
       *
       * @tparam T An object which implements the STL Container concept and stores Operating Points or configurations
       *
       * @param [in] configuration_list The container of the set of Operating Points or configurations to remove
       *
       * @return The number of Operating Points actually removed from the AS-RTM
       *
       * @details
       * This methods remove the target Operating Points from the knowledge base and from all
       * the available states in the AS-RTM.
       * Before removing an Operating Point from the knowledge base, it checks if there is
       * in the knowledge base an Operating Point with the same configuration.
       * Two configurations are the same if all the software knobs have the same mean value.
       * The returned number indicates how many Operating Points are really removed from the
       * knowledge base.
       *
       * The container T must hold object with types OperatingPoint, OperatingPointPtr or
       * OperatingPoint::configuration_type (the type of the configuration segment).
       * Any STL container can be used, however, the best practice is to use a container
       * which is fast to iterate, such as a std::vector< OperatingPoint >.
       */
      template< class T >
      std::size_t remove_operating_points( const T& configuration_list )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);

        // this is meant to be the stream with the removed Operating Points
        OPStream removed_ops;

        // we assume that all the configuration in the list are in the
        // application knowledge of this Application-Specific RunTime Manger
        removed_ops.reserve(configuration_list.size());

        // loop over them and store the removed Operating Points
        for ( const auto& configuration : configuration_list )
        {
          // make sure to add a really new Operating Point
          const auto op_ptr = knowledge.remove(configuration);

          if (op_ptr)
          {
            removed_ops.emplace_back(op_ptr);
          }
        }

        // update also all the States that we have
        for ( auto& state_pair : application_optimizers )
        {
          state_pair.second.remove_operating_points(removed_ops);
        }

        return removed_ops.size();
      }


      /**
       * @brief Retrieve the number of Operating Point in the application knowledge
       *
       * @return The size of the knowledge base object.
       */
      inline std::size_t get_number_operating_points( void ) const
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);
        return knowledge.size();
      }


      /**
       * @brief Test whether the application knowledge is empty
       *
       * @return True, if the knowledge base is empty
       */
      inline bool is_application_knowledge_empty( void ) const
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);
        return knowledge.empty();
      }




      /******************************************************************
       *  METHODS TO MANAGE THE APPLICATION STATES
       ******************************************************************/


      /**
       * @brief Creates a new state in the AS-RTM
       *
       * @param [in] new_state_id The unique identifier of the new state
       *
       * @details
       * Each state represents an indipendent constrained multi-objective optimization
       * problem, which define the application requirements.
       * This method creates a new "empty" state, i.e. a state without any constraint
       * and a dummy objective function which aims at minimizing the first software knob.
       * However, the new state receives all the available Operating Points and runtime
       * knowledge from the AS-RTM (if any).
       *
       * This methods only creates the target state, it doesn't change the active state.
       * If there is already a state with the same name:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, the method as no effects
       */
      void create_new_state( const state_id_type& new_state_id )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);

        // check if the state is already created
        const auto state_it = application_optimizers.find(new_state_id);

        if ( state_it == application_optimizers.end() )
        {
          // if not, create the new state
          const auto return_value = application_optimizers.emplace( new_state_id,
                                    State< OperatingPoint, priority_type, error_coef_type > {} );
          assert(return_value.second && "Error: AS-RTM unable to create a new state");

          // set the knowledge base and field adaptors
          return_value.first->second.set_knowledge_base(knowledge);
          return_value.first->second.set_knowledge_adaptor(runtime_information);
        }
      }


      /**
       * @brief Remove a state from the AS-RTM
       *
       * @param [in] state_id The unique identifier of the target state to remove
       *
       * @details
       * Each state represents an indipendent constrained multi-objective optimization
       * problem, which define the application requirements.
       * This method removes the state with id state_id from the AS-RTM. If there is
       * no such a state:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, the method as no effects
       */
      void remove_state( const state_id_type& state_id )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);

        // get a reference to the target state
        const auto state_it = application_optimizers.find(state_id);

        // make sure that the state exits and that it is different from the active one
        assert( state_it != current_optimizer && "Error: attempt to delete the active state");

        if ((state_it != application_optimizers.end()) && (state_it != current_optimizer))
        {
          application_optimizers.erase(state_it);
        }
      }


      /**
       * @brief Select the active state of the AS-RTM
       *
       * @param [in] state_id The unique identifier of the target state to select
       *
       * @details
       * Each state represents an indipendent constrained multi-objective optimization
       * problem, which define the application requirements.
       * This method select the active state of the AS-RTM. This is important, since all
       * the methods to mange constraints and rank, relates only on the active state.
       * If there is no state with the unique identifier state_id:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, it will lead to an undefined behavior for any
       *    any attempt to use a method which affect the current state.
       */
      void change_active_state( const state_id_type& state_id )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);

        // change the active state
        current_optimizer = application_optimizers.find(state_id);
        assert( current_optimizer != application_optimizers.end() && "Error: attempt to switch to a non-existent state");
      }


      /**
       * @brief Retrieve the id of the current active state
       *
       * @return The value of the unique identifier of the current state
       *
       * @details
       * Each state represents an indipendent constrained multi-objective optimization
       * problem, which define the application requirements.
       * This method retrieves the id of the actual active state. If there are no states,
       * or no state as been previously selected:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, it will lead to an undefined behavior
       */
      inline state_id_type which_active_state( void ) const
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);
        assert( current_optimizer != application_optimizers.end() && "Error: get the id of a non-existent state");
        return current_optimizer->first;
      }




      /******************************************************************
       *  METHODS TO MANAGE RUNTIME INFORMATION
       ******************************************************************/


      /**
       * @brief Add a runtime information provider to the AS-RTM
       *
       * @tparam target_segment The enumerator value of the target segment of the Operating Point
       * @tparam target_field_index The index of the target field within the target segment
       * @tparam inertia The value of the inertia of the field adaptor @see OneSigmaAdaptor
       * @tparam T The type of elements stored in the monitor
       * @tparam statistical_t The type used to compute statistical properties in the monitor
       *
       * @see Monitor
       *
       * @param [in] monitor The monitor which provides runtime information
       *
       * @details
       * This method uses the observation coming from the target monitor to adapt one field
       * of the application knowledge with respect to the current behavior of the application.
       * All the contraints that targets the same field will benefits from the information of
       * the monitor.
       * This information is automatically propagated to all the available states.
       *
       * @note
       * To prevent bias value, every time the AS-RTM selects a differnt configuration for the
       * application, it will clear the observation window of the monitor.
       */
      template< OperatingPointSegments target_segment,
                std::size_t target_field_index,
                std::size_t inertia,
                class T,
                typename statistical_t >
      void add_runtime_knowledge( const Monitor<T, statistical_t>& monitor )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);

        // set this information to the knowledge adaptor
        runtime_information.template emplace< target_segment, target_field_index, inertia, T, statistical_t >(monitor);

        // insert in the monitor buffer the lambda that clear the observations
        auto monitor_buffer = monitor.get_buffer();
        monitor_handlers.emplace_back(
          [monitor_buffer] ( void )   // since we have copied the buffer, it is safe to use (shared_ptr)
        {
          monitor_buffer->clear();
        }
        );

        // we have to update all the states
        for ( auto& state_pair : application_optimizers )
        {
          state_pair.second.set_knowledge_adaptor(runtime_information);
        }
      }


      /**
       * @brief Remove all the runtime information provider
       *
       * @details
       * This method removes all the monitors previously added with the add_runtime_knowledge
       * method. This change is automatically propagated to all the available states.
       */
      void remove_all_runtime_knowledge( void )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);

        // remove all the informations
        runtime_information.clear();

        // remove all the monitor handlers
        monitor_handlers.clear();

        // we have to update all the states
        for ( auto& state_pair : application_optimizers )
        {
          state_pair.second.set_knowledge_adaptor();
        }
      }




      /******************************************************************
       *  METHODS TO COMPUTE AND RETRIEVE THE BEST OPERATING POINT
       ******************************************************************/


      /**
       * @brief Solve the constrained multi-objective optimization problem of the active state
       *
       * @details
       * If the internal state of the AS-RTM is qual to TUNED, it uses runtime information
       * to update the application knowledge, before solving the problem.
       * Regardless of the internal state, any change on the value of the goal of the
       * constraints is taken into account before solving the optimization problem.
       * If there are no states or no state as been previously selected, this method
       * as no effects.
       */
      void find_best_configuration( void )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);

        // check if we need to take into account runtime information
        if ( status == ApplicationStatus::TUNED )
        {
          runtime_information.evaluate_error(application_configuration);
        }

        // solve the actual optimization problem
        if (current_optimizer != application_optimizers.end())
        {
          proposed_best_configuration = current_optimizer->second.get_best_operating_point();
        }
      }


      /**
       * @brief Retrieve the most suitable configuration for the application
       *
       * @return The most suitable configuration for the application
       *
       * @details
       * This method only copy the most suitable configuration, found the last time
       * that we have solved the problem.
       * If the latter is different from the actual configuration of the application,
       * it changes the internal state of the AS-RTM to UNDEFINED, since we don't
       * know when the application apply the suggested configuration.
       *
       * @note
       * If the are no states or no Operating Point, calling this method is
       * considered undefined behavior.
       */
      configuration_type get_best_configuration( void )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);
        assert(proposed_best_configuration && "Error: AS-RTM attempt to retrieve the best configuration from an empty state");

        if (proposed_best_configuration != application_configuration)
        {
          status = ApplicationStatus::UNDEFINED;
        }

        return proposed_best_configuration->get_knobs();
      }


      /**
       * @brief Notify the AS-RTM that the application has applied the suggested configuration
       *
       * @details
       * This method change the internal state of the AS-RTM to TUNED, since the application
       * has explicitly notified the fact that it is using the latest configuration.
       * If the latest configuration was different with respect the previous one, this method
       * clear the observation window of all the monitors related with the AS-RTM.
       */
      void configuration_applied( void )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);

        // check if we need to issue a clear on the monitor
        if (application_configuration != proposed_best_configuration)
        {
          for ( const auto& clear_monitor : monitor_handlers )
          {
            clear_monitor();
          }
        }

        // eventually, set the application configuration as the proposed one
        application_configuration = proposed_best_configuration;
        status = ApplicationStatus::TUNED;
      }




      /******************************************************************
       *  CONSTRAINTS MANAGEMENT
       ******************************************************************/


      /**
       * @brief Add a constraint on the current active state.
       *
       * @tparam segment The enumerator value of the target segment of the Operating Point
       * @tparam field_index The index of the target field within the target segment
       * @tparam sigma The confidence of the constraints, i.e. the number of times the standard deviation is taken into account
       * @tparam ConstraintGoal The type of the constraint's goal
       *
       * @param [in] goal_value The goal that represents the constraint
       * @param [in] priority The priority of the new constraint
       *
       * @see State
       *
       * @details
       * This method insert a new constraint in the current active state. If there are no states,
       * or no state as been previously selected:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, it will lead to an undefined behavior
       */
      template< OperatingPointSegments segment, std::size_t field_index, int sigma, class ConstraintGoal >
      void add_constraint( const ConstraintGoal& goal_value, const priority_type priority )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);

        assert( current_optimizer != application_optimizers.end()
                && "Error: AS-RTM attempt to add a Constraint when there is no active state");
        current_optimizer->second.template add_constraint< segment, field_index, sigma, ConstraintGoal >
        (goal_value, priority, knowledge, runtime_information);
      }


      /**
       * @brief Remove a constraint from the current active state.
       *
       * @param [in] priority The priority of the target constraint to remove
       *
       * @see State
       *
       * @details
       * This method remove a constraint from the current active state. If there are no states,
       * or no state as been previously selected:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, it will lead to an undefined behavior
       */
      void remove_constraint( const priority_type priority )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);

        assert( current_optimizer != application_optimizers.end()
                && "Error: AS-RTM attempt to remove a Constraint when there is no active state");
        current_optimizer->second.remove_constraint(priority);
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
       * @see State
       *
       * @details
       * This method set the objective function of the current active state. If there are no states,
       * or no state as been previously selected:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, it will lead to an undefined behavior
       */
      template< RankObjective objective, FieldComposer composer, class ...Fields >
      void set_rank( Fields ...values )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);

        assert( current_optimizer != application_optimizers.end()
                && "Error: AS-RTM attempt to set the Rank when there is no active state");
        current_optimizer->second.template set_rank< objective, composer, Fields... > ( values... );
      }


    private:


      /**
       * @brief The whole application knowledge
       */
      Knowledge< OperatingPoint > knowledge;

      /**
       * @brief The container that stores the runtime information providers
       */
      KnowledgeAdaptor< OperatingPoint, error_coef_type > runtime_information;

      /**
       * @brief The list of monitor handlers, used to clear their observation window
       */
      MonitorHandlers monitor_handlers;

      /**
       * @brief The container of all the available states
       */
      StateMap application_optimizers;

      /**
       * @brief An iterator of the StateMap, that represents the current active state
       */
      StateIt current_optimizer;

      /**
       * @brief The mutex used to enforce a consistent interal state
       */
      mutable std::mutex manger_mutex;

      /**
       * @brief A pointer to the Operating Point used by the application
       */
      OperatingPointPtr application_configuration;

      /**
       * @brief A pointer to the latest most suitable Operating Point, found by the framework
       */
      OperatingPointPtr proposed_best_configuration;

      /**
       * @brief The internal state of the AS-RTM
       */
      ApplicationStatus status;

  };

}

#endif // MARGOT_ASRTM_HDR
