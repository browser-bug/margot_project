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
#include <iostream>

#include "margot/operating_point.hpp"
#include "margot/knowledge_base.hpp"
#include "margot/knowledge_adaptor.hpp"
#include "margot/monitor.hpp"
#include "margot/state.hpp"
#include "margot/debug.hpp"
#include "margot/margot_config.hpp"

#ifdef MARGOT_WITH_AGORA
  #include <thread>
  #include <sstream>
  #include <chrono>
  #include <ctime>
  #include "agora/virtual_channel.hpp"
  #include "agora/paho_remote_implementation.hpp"
#endif // MARGOT_WITH_AGORA

namespace margot
{

  /**
   * @brief forward the declaration of the Data-Aware Application-Specific RunTime Manager
   */
  template< class Asrtm, typename T, FeatureDistanceType distance_type, FeatureComparison... cfs >
  class DataAwareAsrtm;

  /**
   * @brief The Application-Specific RunTime Manger implementation
   *
   * @tparam OperatingPoint The type which defines the Operating Point characteristics
   * @tparam state_id_type The type used to identify a state
   * @tparam priority_type The type used to represents the priority of a constraint
   * @tparam error_coef_type The type used to compute the error coefficient of the application knowledge
   *
   * @details
   * This class represent the interface of the mARGOt dynamic autotuner toward the appplication, in case of
   * omogenous input, with few abrut changes.
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
       * @brief Explicit definition of this Asrtm type
       */
      using type = Asrtm<OperatingPoint, state_id_type, priority_type, error_coef_type>;


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
        TUNED,


        /**
         * @details
         * In this state, the application is exploring a configuration, typically using the agora
         * remote application handler. While in this state, the values of the metrics of the target
         * Operating Points are meaningless, therefore there is no need to use the information providers
         */
        DESIGN_SPACE_EXPLORATION,


        /**
         * @details
         * In this state, we are no more in the design space exploration, since we have the application
         * knowledge. However, the application doesn't have used it yet, therefore we are not able to
         * tell which is the Operating Point used by the application in the previous run
         */
        WITH_MODEL
      };


      // statically check the template argument
      static_assert(traits::is_operating_point<OperatingPoint>::value,
                    "Error: the Application-Specific RunTime Manager handles object with is_operating_point trait");


    public:


      /**
       * @brief Forward declarion of the Operating Point type
       */
      using operating_point_type = OperatingPoint;


      /**
       * @brief Forward declaration of the priority type
       */
      using priority_type_type = priority_type;


      /**
       * @brief Forward declaration of the state type
       */
      using state_id_type_type = state_id_type;


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

#ifdef MARGOT_WITH_AGORA

      /**
       * @brief Default destructor
       *
       * @details
       * When the AS-RTM interacts directly with the remote application handler,
       * it uses a separate thread for the synchronization.
       * This method ensures that when the manager is destroyed, it gracefully
       * ends the connecction with MQTT and it joins the thread.
       */
      ~Asrtm( void )
      {
        if (local_handler.joinable())
        {
          remote.destroy_channel();
          local_handler.join();
        }
      }
#endif // MARGOT_WITH_AGORA


      /**
       * @brief Copy constructor
       *
       * @param [in] other The source object to be copied from
       */
      Asrtm( const Asrtm& other )
      {
        // it is safe to copy-construct the container
        knowledge = Knowledge<OperatingPoint>(other.knowledge);
        runtime_information = KnowledgeAdaptor<OperatingPoint, error_coef_type>(other.runtime_information);
        monitor_handlers = MonitorHandlers(other.monitor_handlers);
        application_optimizers = StateMap(other.application_optimizers);

        // but extra care should be placed for the iterator of the current state
        current_optimizer = application_optimizers.begin();

        for ( auto it = other.application_optimizers.begin(); it != other.application_optimizers.end(); ++it)
        {
          if (it == other.current_optimizer)
          {
            break;
          }
          else
          {
            ++current_optimizer;
          }
        }

        // it is safe to copy-construct the status and pointers to Operating Points
        application_configuration = other.application_configuration;
        proposed_best_configuration = other.proposed_best_configuration;
        status = other.status;
      }


      /**
       * @brief Move constructor
       *
       * @param [in] other The source object to be copied from
       */
      Asrtm( Asrtm&& other )
      {
        // it is safe to copy-construct the container
        knowledge = std::move(Knowledge<OperatingPoint>(other.knowledge));
        runtime_information = std::move(KnowledgeAdaptor<OperatingPoint, error_coef_type>(other.runtime_information));
        monitor_handlers = std::move(MonitorHandlers(other.monitor_handlers));
        application_optimizers = std::move(StateMap(other.application_optimizers));

        // but extra care should be placed for the iterator of the current state
        current_optimizer = application_optimizers.begin();

        for ( auto it = other.application_optimizers.begin(); it != other.application_optimizers.end(); ++it)
        {
          if (it == other.current_optimizer)
          {
            break;
          }
          else
          {
            ++current_optimizer;
          }
        }

        // it is safe to copy-construct the status and pointers to Operating Points
        application_configuration = other.application_configuration;
        proposed_best_configuration = other.proposed_best_configuration;
        status = other.status;
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
        std::lock_guard< std::mutex > lock(manager_mutex);

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
        std::lock_guard< std::mutex > lock(manager_mutex);

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
        std::lock_guard< std::mutex > lock(manager_mutex);
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
        std::lock_guard< std::mutex > lock(manager_mutex);
        return knowledge.empty();
      }


      /**
       * @brief Test whether the status of the manager is in design space exploration
       *
       * @return True, if we are performing a design space exploration
       */
      inline bool in_design_space_exploration( void ) const
      {
        std::lock_guard< std::mutex > lock(manager_mutex);
        return status == ApplicationStatus::DESIGN_SPACE_EXPLORATION || status == ApplicationStatus::WITH_MODEL;
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
        std::lock_guard< std::mutex > lock(manager_mutex);

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
        std::lock_guard< std::mutex > lock(manager_mutex);

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
      inline void change_active_state( const state_id_type& state_id )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manager_mutex);

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
        std::lock_guard< std::mutex > lock(manager_mutex);
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
        std::lock_guard< std::mutex > lock(manager_mutex);

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
        std::lock_guard< std::mutex > lock(manager_mutex);

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
        std::lock_guard< std::mutex > lock(manager_mutex);

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
       * @param [out] configuration_changed If true, the new configuration is different wrt the previous one
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
      configuration_type get_best_configuration( bool* configuration_changed = nullptr )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manager_mutex);
        assert(proposed_best_configuration && "Error: AS-RTM attempt to retrieve the best configuration from an empty state");

        if (proposed_best_configuration != application_configuration)
        {
          if (configuration_changed != nullptr)
          {
            *configuration_changed = true;
          }

          if ( status != ApplicationStatus::DESIGN_SPACE_EXPLORATION )
          {
            status = ApplicationStatus::UNDEFINED;
          }
        }
        else
        {
          if (configuration_changed != nullptr)
          {
            *configuration_changed = false;
          }
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
        std::lock_guard< std::mutex > lock(manager_mutex);

        // check if we need to issue a clear on the monitor
        if ((application_configuration != proposed_best_configuration) && (status != ApplicationStatus::DESIGN_SPACE_EXPLORATION))
        {
          for ( const auto& clear_monitor : monitor_handlers )
          {
            clear_monitor();
          }
        }

        // eventually, set the application configuration as the proposed one
        application_configuration = proposed_best_configuration;
        status = status != ApplicationStatus::DESIGN_SPACE_EXPLORATION ? ApplicationStatus::TUNED : ApplicationStatus::DESIGN_SPACE_EXPLORATION;;
      }


      /**
       * @brief Retrieves the mean value of a field of the current Operating Point
       *
       * @tparam segment The target segment of the Operating Point
       * @tparam field The index of the target field wihin the segment
       * @tparam T The type of the return value
       *
       * @return The mean value of the requested field
       *
       * @details
       * This method evaluates the last Operating Point used by the application,
       * not a possible new one, found by the autotuner.
       * The idea is that this method retrieves the expected value of the target field
       *
       * @note
       * Using this method without Operating Point causes undefined behavior
       */
      template< OperatingPointSegments segment, std::size_t field, class T = float >
      inline T get_mean( void ) const
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manager_mutex);
        assert(application_configuration && "Error: AS-RTM attempt to retrieve information from an empty application configutation");
        return application_configuration ? static_cast<T>(Evaluator< OperatingPoint, FieldComposer::SIMPLE,
               OPField< segment, BoundType::LOWER, field, 0> >::evaluate(application_configuration)) : static_cast<T>(9999);
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
        std::lock_guard< std::mutex > lock(manager_mutex);

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
        std::lock_guard< std::mutex > lock(manager_mutex);

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
        std::lock_guard< std::mutex > lock(manager_mutex);

        assert( current_optimizer != application_optimizers.end()
                && "Error: AS-RTM attempt to set the Rank when there is no active state");
        current_optimizer->second.template set_rank< objective, composer, Fields... > ( values... );
      }




      /******************************************************************
       *  UTILITY FUNCTION FOR THE DATA-AWARE AS-RTM
       ******************************************************************/


      /**
       * @brief Create a pseudo-copy of the current AS-RTM
       *
       * @return The created sibling object
       *
       * @details
       * This method is used by the Data-Aware AS-RTM to create a new feature cluster as a copy
       * of this AS-RTM, but without any information about the application knowledge.
       * Moreover, they must be independent, without sharing any data-structure.
       */
      inline Asrtm<OperatingPoint, state_id_type, priority_type, error_coef_type> create_sibling( void ) const
      {
        // create a new Application-Specific Run-Time Manager
        Asrtm<OperatingPoint, state_id_type, priority_type, error_coef_type> cloned_manager;


        // replicate the state structure
        for ( const auto& state : application_optimizers )
        {
          // insert the optimization problem
          const auto result = cloned_manager.application_optimizers.emplace(state.first, state.second.create_sibling());

          // check if it is the current state
          if (state.first == result.first->first)
          {
            assert(result.second && "Error: something went wrong on creating the AS-RTM sibling");
            cloned_manager.current_optimizer = result.first;
          }
        }

        // copy the runtime information provider
        cloned_manager.runtime_information = runtime_information;

        // reset their values
        cloned_manager.runtime_information.reset();

        // copy the monitor handlers
        cloned_manager.monitor_handlers = monitor_handlers;

        return cloned_manager;
      }


      /**
       * @brief A soft reset of the AS-RTM
       *
       * @details
       * This method forces this AS-RTM to notify the application that a new configuration
       * has been found. It is used by the Data-Aware AS-RTM when the application switch to
       * a different feature cluster
       */
      inline void restore_from_data_feature_switch( void )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manager_mutex);
        status = status != ApplicationStatus::DESIGN_SPACE_EXPLORATION ? ApplicationStatus::UNDEFINED : ApplicationStatus::DESIGN_SPACE_EXPLORATION;;
        application_configuration.reset();
      }




      /******************************************************************
       *  AGORA LOCAL APPLICATION HANDLER PUBLIC METHODS
       ******************************************************************/


#ifdef MARGOT_WITH_AGORA

      /**
       * @brief Send an observation to the agora remote application handler
       *
       * @param [in] measures The behavior of the application
       *
       * @details
       * This methods sends to the agora remote application handler the current
       * observation of the performance of the application.
       * In particular, the parameter must be composed as following:
       *    - Global structure: "<knobs> <features> <metrics>"
       *    - <knobs>: "value_knob_1,value_knob_2,value_knob_n"
       *    - <features>: "value_f_1,value_f_2,value_f_n"
       *    - <metrics>: "value_m_1,value_m_2,value_m_n"
       * If there are no features, they might be omitted
       */
      inline void send_observation( const std::string& measures )
      {
        std::lock_guard< std::mutex > lock(manager_mutex);

        // get the timestamp of now
        auto now = std::chrono::system_clock::now();

        // convert the measure in seconds since epoch and
        // nanosec since second
        const auto sec_since_now = std::chrono::duration_cast< std::chrono::seconds >(now.time_since_epoch());
        const auto almost_epoch = now - sec_since_now;
        const auto ns_since_sec = std::chrono::duration_cast< std::chrono::nanoseconds >(almost_epoch.time_since_epoch());

        // send the message
        remote.send_message({{"margot/" + application_name + "/observation"}, std::to_string(sec_since_now.count()) + ","
          + std::to_string(ns_since_sec.count()) + " "
          + remote.get_my_client_id() + " "
          + measures
        });
      }

      /**
       * @brief starts the support thread that communicate with the remote application handler
       *
       * @tparam OpConverter The type of a functor that generates an Operating Point from a string
       *
       * @param [in] application The name of the application with format "<name>/<version>/<block>"
       * @param [in] broker_url The address of the MQTT broker
       * @param [in] username The username required to authenticate with the broker. Leave empty if it is not required
       * @param [in] password The passwoed required to authenticate with the broker. Leave empty if it is not required
       * @param [in] qos_level The level of Quality of Service used to communicate with the broker [0,2]
       * @param [in] description The information required by agora to handle the application
       * @param [in] broker_ca The path to the broker Certificate Authority. Leave empty if it is not required
       * @param [in] client_cert The path to the client certificate. Leave empty if it is not required
       * @param [in] client_key The path to the client provate key. Leave empty if it is not required
       *
       * @details
       * The format of the description follow these rules:
       *   - The description is row base, where each row specify an information
       *   - The character '@' separates the lines of the description
       *   - The first 10 characters identify the type of the row
       *   - In the current implementation, the description might be composed as follows:
       *      - "metric    <metric_name> <metric_type> <prediction_method>"
       *      - "knob      <knob_name> <knob_type> <values>"
       *      - "feature   <feature_name> <feature_type> <values>"
       *      - "doe       <name_of_doe_strategy>"
       *      - "num_obser <num_observations>"
       *   - The field <values> is a coma separated list of values
       */
      template< class OpConverter >
      void start_support_thread( const std::string& application, const std::string& broker_url, const std::string& username, const std::string& password, const int qos_level,
                                 const std::string& description, const std::string& broker_ca, const std::string& client_cert, const std::string& client_key )
      {
        std::lock_guard< std::mutex > lock(manager_mutex);

        // get the application name
        application_name = application;

        // disable the agora logging
        agora::my_agora_logger.set_filter_at(agora::LogLevel::DISABLED);

        // set the status of the autotuner to DSE
        status = ApplicationStatus::DESIGN_SPACE_EXPLORATION;

        // initialize communication channel with the server
        remote.create<agora::PahoClient>(application_name, broker_url, qos_level, username, password, broker_ca, client_cert, client_key );

        // start the thread
        local_handler = std::thread(&type::local_application_handler<OpConverter>, this, description);
      }

#endif // MARGOT_WITH_AGORA




      /******************************************************************
       *  DEBUG METHODS
       ******************************************************************/


      /**
       * @brief This method prints on the standard output the state of the Asrtm
       *
       * @param [in] starting_point States if this call is the first dump call
       *
       * @details
       * Since the exposed interface toward the application is either this class
       * or the Data-Aware AS-RTM, the parameters is used to check whether we
       * should print the header or not.
       */
      void dump( bool starting_point = true ) const;


    private:



      /******************************************************************
       *  AGORA LOCAL APPLICATION HANDLER PRIVATE METHODS
       ******************************************************************/


#ifdef MARGOT_WITH_AGORA


      /**
       * @brief Replace the current knowledge base with a single point
       *
       * @param [in] point The input Operating Point
       */
      void set_sinlgle_point( const OperatingPoint& point )
      {
        // lock the asrtm, since we are changing its data structure
        std::lock_guard<std::mutex> lock(manager_mutex);

        // clear the knowledge base from previous values
        knowledge.clear();

        // add the Operating Point to the knowledge
        knowledge.add(point);

        // set the new knowledge to all the states
        for ( auto& state_pair : application_optimizers )
        {
          state_pair.second.set_knowledge_base(knowledge);
        }

        // reset the state of the asrtm
        status = ApplicationStatus::DESIGN_SPACE_EXPLORATION;
        proposed_best_configuration.reset();
      }


      /**
       * @brief Force the state of the Asrtm to Design Space Exploration
       *
       * @details
       * This method is supposed to be called by DA-ASRTM once it starts
       * the local application handler.
       */
      inline void set_autotuner_in_dse( void )
      {
        // lock the asrtm, since we are changing its status
        std::lock_guard<std::mutex> lock(manager_mutex);
        status = ApplicationStatus::DESIGN_SPACE_EXPLORATION;
      }


      /**
       * @brief Replace the current knowledge base with the given Operating Point list
       *
       * @param [in] model The input Operating Points list
       *
       * @details
       * To ensure a fresh start with the new list of Operating Points, this function
       * resets all the information providers.
       */
      void set_model( const std::vector< OperatingPoint >& model )
      {
        // lock the asrtm, since we are changing its data structure
        std::lock_guard<std::mutex> lock(manager_mutex);

        // clear the knowledge base from previous values
        knowledge.clear();

        // add the Operating Point to the knowledge
        for ( const auto& op : model )
        {
          knowledge.add(op);
        }

        // set the new knowledge to all the states
        for ( auto& state_pair : application_optimizers )
        {
          state_pair.second.set_knowledge_base(knowledge);
        }

        // reset the information providers
        runtime_information.reset();

        // reset the state of the asrtm
        status = ApplicationStatus::WITH_MODEL;
        proposed_best_configuration.reset();
      }


      /**
       * @brief The function executed by the agora local application handler
       *
       * @tparam OpConverter The type of a functor that generates an Operating Point from a string
       *
       * @param [in] application_description The string that describes the application for agora
       * @see start_support_thread
       *
       * @details
       * This method loops until the communication channel with the broker is destroyed and
       * process all the messages coming from the remote application handler.
       * This method is designed to be executed from a separate thread, which destruction is
       * forced by the destructor of this class.
       */
      template< class OpConverter >
      void local_application_handler( const std::string application_description )
      {
        agora::info("mARGOt support thread on duty");

        // declare the the object used to get an Operating Point from a string
        OpConverter get_op;

        // get my own id
        const std::string my_client_id = remote.get_my_client_id();

        // register to the application-specific topic (before send the welcome message)
        remote.subscribe("margot/" + application_name + "/" + my_client_id + "/#");

        // register to the application topic to receive the model
        remote.subscribe("margot/" + application_name + "/model");

        // register to the server welcome topic
        remote.subscribe("margot/agora/welcome");

        // announce to the world that i exist
        remote.send_message({{"margot/" + application_name + "/welcome"}, my_client_id});

        // remember, this is a thread, it should terminate only when the
        // MQTT client disconnect, so keep running
        while (true)
        {
          // declaring the new message
          agora::message_t new_incoming_message;

          if (!remote.recv_message(new_incoming_message))
          {
            agora::info("mARGOt support thread on retirement");
            return; // there is no more work available
          }

          // get the "topic" of the message
          const auto start_type_pos = new_incoming_message.topic.find_last_of('/');
          const std::string message_topic = new_incoming_message.topic.substr(start_type_pos);


          // handle the single configurations coming from the server
          if (message_topic.compare("/explore") == 0)
          {
            set_sinlgle_point(get_op(new_incoming_message.payload));
          }
          else  if (message_topic.compare("/info") == 0)   // handle the info message
          {
            remote.send_message({{"margot/" + application_name + "/info"}, application_description});
          }
          else if (message_topic.compare("/model") == 0) // handle the final model coming from the server
          {
            // parse the the incoming message with the model
            std::vector< OperatingPoint > model;
            std::stringstream model_stream(new_incoming_message.payload);
            constexpr char line_delimiter = '@';
            std::string op_string;

            while (std::getline(model_stream, op_string, line_delimiter))
            {
              std::string knobs;
              std::string metrics;
              std::stringstream op_stream(op_string);
              op_stream >> knobs;
              op_stream >> metrics;
              model.emplace_back(get_op(knobs, metrics));
            }

            // set the model
            set_model(model);
          }
          else if (message_topic.compare("/welcome") == 0) // handle the case where a new agora handler appears
          {
            // send a welcome message to restore the communication
            remote.send_message({{"margot/" + application_name + "/welcome"}, my_client_id});
          }
        }
      }

      /**
       * @brief Declare the DataAwareAsrtm as a friend class
       *
       * @details
       * The Data-Aware Application-Specific Run-Time Manager needs to set a single Operating Point
       * or a list to handle the application using agora
       */
      template< class Asrtm, typename T, FeatureDistanceType distance_type, FeatureComparison... cfs >
      friend class DataAwareAsrtm;

#endif // MARGOT_WITH_AGORA


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
      mutable std::mutex manager_mutex;

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

#ifdef MARGOT_WITH_AGORA

      /**
       * @brief The handler of the local agora application handler
       */
      std::thread local_handler;

      /**
       * @brief This is the virtual channel used to communicate with the agora remote application handler
       */
      agora::VirtualChannel remote;

      /**
       * @brief The name of this application
       */
      std::string application_name;

#endif // MARGOT_WITH_AGORA

  };


  template< class OperatingPoint, class state_id_type, typename priority_type, typename error_coef_type >
  void Asrtm<OperatingPoint, state_id_type, priority_type, error_coef_type>::dump( bool starting_point ) const
  {
    // check if we have to print the header
    if (starting_point)
    {
      print_header();
    }

    // print the total recap about the state
    std::cout << "# ///////////////////////////////////////////////////////////////////" << std::endl;
    std::cout << "# //" << std::endl;
    std::cout << "# // Application-Specific RunTime Manager status dump" << std::endl;
    std::cout << "# //" << std::endl;

    // print the summary about the manager
    std::cout << "# // Number of Operating Points in the knowledge: " << knowledge.size() << std::endl;
    std::cout << "# // Number of State(s) (optimization problems): " << application_optimizers.size() << std::endl;
    std::cout << "# // Synchronization with application: ";

    if ( status == ApplicationStatus::UNDEFINED )
    {
      std::cout << "OUT OF SYNC" << std::endl;
    }
    else
    {
      if ( status == ApplicationStatus::TUNED )
      {
        std::cout << "SYNC" << std::endl;
      }
      else
      {
        std::cout << "UNKNOWN (ERROR!)" << std::endl;
      }
    }

    const bool is_active_state_selected = current_optimizer != application_optimizers.end();

    if (is_active_state_selected)
    {
      std::cout << "# // Active State address: " << &current_optimizer->second << std::endl;
      std::cout << "# // Active State id: " << current_optimizer->first << std::endl;
    }
    else
    {
      std::cout << "# // Active State address: N/A" << std::endl;
      std::cout << "# // Active State id: N/A" << std::endl;
    }

    std::cout << "# //" << std::endl;



    // print the last known configuration of the application
    std::cout << "# // ----------------------------------------------------------" << std::endl;
    std::cout << "# // -- Last known configuration of the application" << std::endl;
    std::cout << "# // ----------------------------------------------------------" << std::endl;

    std::cout << "# //" << std::endl;

    if (application_configuration)
    {
      print_whole_op<OperatingPoint>(application_configuration, "# //");
    }
    else
    {
      std::cout << "# // Actually we have no clue, sorry :(" << std::endl;
    }

    std::cout << "# //" << std::endl;


    // print the last known configuration of the application
    std::cout << "# // ----------------------------------------------------------" << std::endl;
    std::cout << "# // -- The proposed best configuration for the application" << std::endl;
    std::cout << "# // ----------------------------------------------------------" << std::endl;

    std::cout << "# //" << std::endl;

    if (proposed_best_configuration)
    {
      print_whole_op<OperatingPoint>(proposed_best_configuration, "# //");
    }
    else
    {
      std::cout << "# // No information about it, maybe we have to find it before..." << std::endl;
    }

    std::cout << "# //" << std::endl;




    // print the application knwoledge
    std::cout << "# // ----------------------------------------------------------" << std::endl;
    std::cout << "# // -- Application knwoledge dump" << std::endl;
    std::cout << "# // ----------------------------------------------------------" << std::endl;

    // get the whole knowledge as stream
    OPStream kb_stream = knowledge.to_stream();

    // print the ops
    for ( const auto& op : kb_stream )
    {
      std::cout << "# //" << std::endl;
      print_whole_op<OperatingPoint>(op, "# //");
      std::cout << "# //" << std::endl;
    }

    if (kb_stream.empty())
    {
      std::cout << "# //" << std::endl;
      std::cout << "# // No Operating Points in the knowledge" << std::endl;
      std::cout << "# //" << std::endl;
    }



    // print the optimization states
    std::cout << "# // ----------------------------------------------------------" << std::endl;
    std::cout << "# // -- Runtume information providers" << std::endl;
    std::cout << "# // ----------------------------------------------------------" << std::endl;

    // print the number of monitor resetters
    std::cout << "# //" << std::endl;
    std::cout << "# // Number of monitor handlers: " << monitor_handlers.size() << std::endl;

    // loop over them
    runtime_information.dump("# //");




    // print the optimization states
    std::cout << "# // ----------------------------------------------------------" << std::endl;
    std::cout << "# // -- List of optimization problems available (states)" << std::endl;
    std::cout << "# // ----------------------------------------------------------" << std::endl;

    // loop over them
    for ( auto it = application_optimizers.begin(); it != application_optimizers.end(); ++it )
    {
      std::cout << "# //" << std::endl;
      const std::string suffix = it == current_optimizer ? " <---- CURRENT STATE " : "";
      std::cout << "# // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl;
      std::cout << "# // @       STATE " << it->first << suffix << std::endl;
      std::cout << "# // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl;
      std::cout << "# // @" << std::endl;

      // dump the status of the state
      it->second.dump("# // @");

      std::cout << "# // @" << std::endl;
      std::cout << "# // @" << std::endl;
      std::cout << "# // @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@" << std::endl;
      std::cout << "# //" << std::endl;
    }


    if (application_optimizers.empty())
    {
      std::cout << "# //" << std::endl;
      std::cout << "# // No Optimization states available" << std::endl;
      std::cout << "# //" << std::endl;
    }



    // print the final recap
    std::cout << "# //" << std::endl;
    std::cout << "# ///////////////////////////////////////////////////////////////////" << std::endl;


    // check if we have to print the trailer
    if (starting_point)
    {
      print_trailer();
    }
  }

}

#endif // MARGOT_ASRTM_HDR
