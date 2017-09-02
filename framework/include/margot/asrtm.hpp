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


  template< class OperatingPoint, class state_id_type = std::string, typename priority_type = int, typename error_coef_type = float >
  class Asrtm
  {

      enum class ApplicationStatus
      {
        UNDEFINED,
        TUNED
      };


      // statically check the template argument
      static_assert(traits::is_operating_point<OperatingPoint>::value,
                    "Error: the Application-Specific RunTime Manager handles object with is_operating_point trait");


    public:

      /**
       * @brief Explicit definition to an Operating Point pointer
       */
      using OperatingPointPtr = std::shared_ptr<OperatingPoint>;

      /**
       * @brief Definition of the stream of Operating Point
       */
      using OPStream = std::vector< OperatingPointPtr >;

      using StateMap = std::unordered_map< state_id_type, State< OperatingPoint, priority_type, error_coef_type > >;

      using StateIt = typename StateMap::iterator;

      using configuration_type = typename OperatingPoint::configuration_type;

      using clear_monitor_function_type = std::function<void(void)>;

      using MonitorHandlers = std::vector< clear_monitor_function_type >;


      Asrtm( void ): status(ApplicationStatus::UNDEFINED)
      {
        current_optimizer = application_optimizers.end();
      }


      /******************************************************************
       *  METHODS TO MANAGE THE APPLICATION KNOWLEDGE
       ******************************************************************/


      // T is a stl container of either OP or pointer to OP
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

        // take the number of the really new Operating Points
        const std::size_t number_new_ops = new_ops.size();

        // update also all the States that we have
        for ( auto& state_pair : application_optimizers )
        {
          state_pair.second.add_operating_points(new_ops);
        }

        return number_new_ops;
      }


      // T is a stl container of configurations
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

        // take the number of removed Operating Points
        const std::size_t number_removed_ops = removed_ops.size();

        // update also all the States that we have
        for ( auto& state_pair : application_optimizers )
        {
          state_pair.second.remove_operating_points(removed_ops);
        }

        return number_removed_ops;
      }

      inline std::size_t get_number_operating_points( void ) const
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);
        return knowledge.size();
      }

      inline bool is_application_knowledge_empty( void ) const
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);
        return knowledge.empty();
      }


      /******************************************************************
       *  METHODS TO MANAGE THE APPLICATION STATES
       ******************************************************************/

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

      void change_active_state( const state_id_type& state_id )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);

        // change the active state
        current_optimizer = application_optimizers.find(state_id);
        assert( current_optimizer != application_optimizers.end() && "Error: attempt to switch to a non-existent state");
      }

      state_id_type which_active_state( void ) const
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(manger_mutex);
        return current_optimizer->first;
      }


      /******************************************************************
       *  METHODS TO MANAGE RUNTIME INFORMATION
       ******************************************************************/

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

      Knowledge< OperatingPoint > knowledge;
      KnowledgeAdaptor< OperatingPoint, error_coef_type > runtime_information;
      MonitorHandlers monitor_handlers;


      StateMap application_optimizers;
      StateIt current_optimizer;

      mutable std::mutex manger_mutex;


      OperatingPointPtr application_configuration;
      OperatingPointPtr proposed_best_configuration;

      ApplicationStatus status;
  };








}

#endif // MARGOT_ASRTM_HDR
