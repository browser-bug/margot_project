/* core/da_asrtm.hpp
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

#ifndef MARGOT_DA_ASRTM_HDR
#define MARGOT_DA_ASRTM_HDR

#include <memory>
#include <string>
#include <map>
#include <vector>
#include <cstddef>
#include <cassert>
#include <mutex>
#include <functional>

#include "margot/operating_point.hpp"
#include "margot/asrtm.hpp"


namespace margot
{

  /**
   * @brief The Data Aware Application-Specific RunTime Manger implementation
   *
   * @tparam OperatingPoint The type which defines the Operating Point characteristics
   * @tparam Feature The type of the considered data feature
   * @tparam Compare Function object for performing comparisons. Unless specialized, invokes operator< on type Feature
   * @tparam state_id_type The type used to identify a state
   * @tparam priority_type The type used to represents the priority of a constraint
   * @tparam error_coef_type The type used to compute the error coefficient of the application knowledge
   *
   * @see Asrtm
   *
   * @details
   * This class represent the interface of the mARGOt dynamic autotuner toward the appplication, if we want to
   * take in consideration heterogeneous input, without any assumption on the order of the input.
   * The idea of this class is to allocate a separate Asrtm for each cluster of input, represented by the Feature
   * template parameter.
   * At run-time the application set the correct data feature for the actual input, and this class will use that value
   * for selecting the most suitable configuration.
   * All the methods that defines the optimization problem, e.g. adding a constraint, will apply to all the Input
   * features. The other methods affects only the "active" Asrtm.
   *
   * However, all the methods are mutex protected, to enforce a consistent internal state.
   *
   * @note
   * The state_id_type and priority_type types require that std::hash is able to compute their hash value.
   * It must be defined the < operator for priority type.
   * The error_coef_type must be a floating point aritmethic type.
   * The internal representation uses a Feature object as key in a std::map. If Compare is not specified, the
   * type Feature must implement the < operator. Otherwise it is possible to define a custom order.
   * To select the correct data feature cluster it uses the lower_bound method.
   * In any case the Feature type must implement the == operator.
   */
  template< class OperatingPoint, class Feature = int, class Compare = std::less<Feature>, class state_id_type = std::string, typename priority_type = int, typename error_coef_type = float >
  class DataAwareAsrtm
  {



      // statically check the template argument
      static_assert(traits::is_operating_point<OperatingPoint>::value,
                    "Error: the Data-Aware Application-Specific RunTime Manager handles object with is_operating_point trait");


    public:


      /**
       * @brief Explicit definition to an Operating Point pointer
       */
      using OperatingPointPtr = typename Knowledge<OperatingPoint>::OperatingPointPtr;


      /**
       * @brief The definition of the container of the Asrtm
       */
      using AsrtmContainer = std::map< Feature, Asrtm< OperatingPoint, state_id_type, priority_type, error_coef_type >, Compare >;


      /**
       * @brief Definition of the stream of Operating Point
       *
       * @details
       * This container is used as interface from the other component to find
       * the best OperatingPoint from the container
       */
      using OPStream = typename Knowledge<OperatingPoint>::OPStream;


      /**
       * @brief Explicit definition of the type of the configuration
       */
      using configuration_type = typename OperatingPoint::configuration_type;


      /**
       * @brief Default constructor
       *
       * @details
       * This method creates a data-aware application-specific runtime manager.
       * It is required to instantiate a feature cluster before performing any
       * other operation such as defining a rank or adding a constraint.
       */
      DataAwareAsrtm( void )
      {
        // enforce the actual state of the AS-RTM
        active_manager = managers.end();
      }




      /******************************************************************
       *  METHODS TO MANAGE THE FEATURE CLUSTERS
       ******************************************************************/


      /**
       * @brief Create a niew data feature cluster
       *
       * @param [in] key The Feature object that defines the feature cluster
       *
       * @details
       * The new Application-Specific RunTime manager is either a flat new object if
       * it is the first manager created, otherwise this method uses the first another
       * asrtm to get all states and runtime information.
       * The idea is to keep all the data feature cluster syncronized about the definition
       * of the optimization problem.
       * If key object is already taken by another feature cluster, no actions are performed.
       * Creating a new feature cluster does not have any effect on the current active cluster.
       */
      void add_feature_cluster( const Feature key )
      {
        // make sure that no one else is going to change the container
        std::lock_guard<std::mutex> lock(asrtm_mutex);

        // check if we need to get the optimization problems from another
        // application-specific runtime manager
        const auto first_cluster = managers.begin();
        if (first_cluster != managers.end())
        {
          // emplace the new asrtm (if it's actually new)
          managers.emplace(key, first_cluster->get_sibling());
        }
        else
        {
          // it's the first cluster
          managers.emplace(key, Asrtm< OperatingPoint, state_id_type, priority_type, error_coef_type >{});
        }
      }


      /**
       * @brief Change the active feature cluster
       *
       * @param [in] key The feature value of interest
       *
       * @details
       * This method tries to select the correct feature cluster given the key in
       * input. It uses the lower_bound method of a std::map to retrieve the closer
       * feature cluster. Therefore the implementation depends on the Compare object
       * used to sort the feature clusters.
       * If there are no feature cluster:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, it will lead to an undefined behavior
       */
      void select_feature_cluster( const Feature key )
      {
        // make sure that no one else is going to change the container
        std::lock_guard<std::mutex> lock(asrtm_mutex);

        // check if there is any cluster available
        assert( !managers.empty() && "Error: attempt to select a cluster from an empty container");

        // get the lower bound cluster
        const auto low_cluster = managers.lower_bound(key);

        // if it is the final one, go back
        active_manager = low_cluster != managers.end() ? low_cluster : std::prev(low_cluster);

        // reset the information about the current Operating Point
        active_manager->second.restore_from_data_feature_switch();
      }


      /**
       * @brief Remove the target feature cluster
       *
       * @param [in] key The feature value of the target cluster
       *
       * @details
       * This method deletes the feature cluster with the given key. If no cluster
       * have the same key, no action are performed.
       * It is forbidden to delete the active feature cluster, in that case:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, it will lead to an undefined behavior
       */
      void remove_feature_cluster( const Feature key )
      {
        // make sure that no one else is going to change the container
        std::lock_guard<std::mutex> lock(asrtm_mutex);

        // find the target feature cluster
        const auto result = managers.find(key);

        // make sure that the selected feature cluster is different from the active one
        assert( result != active_manager && "Error: attempt to delete the active feature cluster");

        // if found, delete the selected manager
        if (result != managers.end())
        {
          managers.erase(result);
        }
      }




      /******************************************************************
       *  FORWARD METHODS TO MANAGE THE APPLICATION KNOWLEDGE
       ******************************************************************/


      /**
       * @brief Add Operating Points to the active AS-RTM
       *
       * @tparam T An object which implements the STL Container concept and stores Operating Points
       *
       * @param [in] op_list The container of the set of new Operating Points
       *
       * @return The number of Operating Points actually added to the active AS-RTM
       *
       * @see Asrtm
       *
       * @details
       * This is a forward method for the Asrtm class. This method adds Operating Points only on the
       * active Asrtm. In this way it is possible to have different OPs for each feature cluster.
       * It is forbidden to add Operating Points from a non-defined active cluster, in that case:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, it will lead to an undefined behavior
       */
      template< class T >
      inline std::size_t add_operating_points( const T& op_list )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // make sure that there is an active feature cluster
        assert(active_manager != managers.end() && "Error: attempt to add Operating Points to a non-existent Asrtm");

        // method forward
        return active_manager->second.add_operating_points(op_list);
      }


      /**
       * @brief Remove Operating Points from the active AS-RTM
       *
       * @tparam T An object which implements the STL Container concept and stores Operating Points or configurations
       *
       * @param [in] configuration_list The container of the set of Operating Points or configurations to remove
       *
       * @return The number of Operating Points actually removed from the AS-RTM
       *
       * @see Asrtm
       *
       * @details
       * This is a forward method for the Asrtm class. This method removes Operating Points only on the
       * active Asrtm. In this way it is possible to have different OPs for each feature cluster.
       * It is forbidden to remove Operating Points from a non-defined active cluster, in that case:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, it will lead to an undefined behavior
       */
      template< class T >
      inline std::size_t remove_operating_points( const T& configuration_list )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // make sure that there is an active feature cluster
        assert(active_manager != managers.end() && "Error: attempt to remove Operating Points to a non-existent Asrtm");

        // method forward
        return active_manager->second.remove_operating_points(configuration_list);
      }


      /**
       * @brief Retrieve the number of Operating Point in the application knowledge for the active AS-RTM
       *
       * @return The size of the knowledge base object.
       *
       * @see Asrtm
       *
       * @details
       * It is forbidden to query the number of Operating Points from a non-defined active cluster, in that case:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, it will lead to an undefined behavior
       */
      inline std::size_t get_number_operating_points( void ) const
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // make sure that there is an active feature cluster
        assert(active_manager != managers.end() && "Error: attempt to query the number of Operating Points to a non-existent Asrtm");

        // method forward
        return active_manager->second.get_number_operating_points();
      }


      /**
       * @brief Test whether the application knowledge is empty for the active AS-RTM
       *
       * @return True, if the knowledge base is empty
       *
       * @see Asrtm
       *
       * @details
       * It is forbidden to check if there are Operating Points in a non-defined active cluster, in that case:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, it will lead to an undefined behavior
       */
      inline bool is_application_knowledge_empty( void ) const
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // make sure that there is an active feature cluster
        assert(active_manager != managers.end() && "Error: attempt to test the knowledge base of a non-existent Asrtm");

        // method forward
        return active_manager->second.is_application_knowledge_empty();
      }




      /******************************************************************
       *  FORWARD METHODS TO MANAGE THE APPLICATION STATES
       ******************************************************************/


      /**
       * @brief Creates a new state in the AS-RTM of all the feature clusters
       *
       * @param [in] new_state_id The unique identifier of the new state
       *
       * @see Asrtm
       *
       * @details
       * This is a forward method for the Asrtm. This operation is performed on all
       * the data feature clusters defined.
       */
      inline void create_new_state( const state_id_type& new_state_id )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // loop over the available AS-RTM
        for( auto& asrtm_pair : managers )
        {
          // create a new state
          asrtm_pair.second.create_new_state(new_state_id);
        }
      }


      /**
       * @brief Remove a state from the AS-RTM of all the feature clusters
       *
       * @param [in] state_id The unique identifier of the target state to remove
       *
       * @see Asrtm
       *
       * @details
       * This is a forward method for the Asrtm. This operation is performed on all
       * the data feature clusters defined.
       */
      inline void remove_state( const state_id_type& state_id )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // loop over the available AS-RTM
        for( auto& asrtm_pair : managers )
        {
          // create a new state
          asrtm_pair.second.remove_state(state_id);
        }
      }


      /**
       * @brief Select the active state of the AS-RTM of all the feature clusters
       *
       * @param [in] state_id The unique identifier of the target state to select
       *
       * @see Asrtm
       *
       * @details
       * This is a forward method for the Asrtm. This operation is performed on all
       * the data feature clusters defined.
       */
      inline void change_active_state( const state_id_type& state_id )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // loop over the available AS-RTM
        for( auto& asrtm_pair : managers )
        {
          // create a new state
          asrtm_pair.second.change_active_state(state_id);
        }
      }


      /**
       * @brief Retrieve the id of the current active state in the current active AS-RTM
       *
       * @return The value of the unique identifier of the current state
       *
       * @see Asrtm
       *
       * @details
       * This is a forward method for the Asrtm. The result is the same, regardless of the
       * active feature cluster. This method uses the active cluster to query the id of the
       * active state. Therefore, in case it is not defined the active cluster:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, it will lead to an undefined behavior
       */
      inline state_id_type which_active_state( void ) const
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // make sure that there is an active feature cluster
        assert(active_manager != managers.end() && "Error: attempt to query the active state in a non-existent Asrtm");

        // method forward
        return active_manager->second.which_active_state();
      }




      /******************************************************************
       *  FORWARD METHODS TO MANAGE RUNTIME INFORMATION
       ******************************************************************/


      /**
       * @brief Add a runtime information provider to all the AS-RTM of feature clusters
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
       * @see Asrtm
       *
       * @details
       * This is a forward method for the Asrtm. This operation is performed on all
       * the data feature clusters defined.
       */
      template< OperatingPointSegments target_segment,
                std::size_t target_field_index,
                std::size_t inertia,
                class T,
                typename statistical_t >
      inline void add_runtime_knowledge( const Monitor<T, statistical_t>& monitor )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // loop over the available AS-RTM
        for( auto& asrtm_pair : managers )
        {
          // create a new state
          asrtm_pair.second.template add_runtime_knowledge<target_segment,target_field_index,inertia,T,statistical_t>(monitor);
        }
      }


      /**
       * @brief Remove all the runtime information provider from all the data feature clusters
       *
       * @see Asrtm
       *
       * @details
       * This is a forward method for the Asrtm. This operation is performed on all
       * the data feature clusters defined.
       */
      inline void remove_all_runtime_knowledge( void )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // loop over the available AS-RTM
        for( auto& asrtm_pair : managers )
        {
          // create a new state
          asrtm_pair.second.remove_all_runtime_knowledge();
        }
      }




      /******************************************************************
       *  METHODS TO COMPUTE AND RETRIEVE THE BEST OPERATING POINT
       ******************************************************************/


      /**
       * @brief Solve the constrained multi-objective optimization problem of for the active feature cluster
       *
       * @see Asrtm
       *
       * @details
       * This is a forward method for the Asrtm. This operation is performed only on the active state
       * of the active feature cluster
       * Therefore, in case it is not defined the active cluster:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, it will lead to an undefined behavior
       */
      inline void find_best_configuration( void )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // make sure that there is an active feature cluster
        assert(active_manager != managers.end() && "Error: attempt to find the best configuration from a non-existent Asrtm");

        // method forward
        active_manager->second.find_best_configuration();
      }


      /**
       * @brief Retrieve the most suitable configuration for the application from the active feature cluster
       *
       * @param [out] configuration_changed If true, the new configuration is different wrt the previous one
       *
       * @return The most suitable configuration for the application
       *
       * @see Asrtm
       *
       * @details
       * This is a forward method for the Asrtm. This operation is performed only on the active state
       * of the active feature cluster
       * Therefore, in case it is not defined the active cluster:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, it will lead to an undefined behavior
       */
      inline configuration_type get_best_configuration( bool* configuration_changed = nullptr )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // make sure that there is an active feature cluster
        assert(active_manager != managers.end() && "Error: attempt to get the best configuration from a non-existent Asrtm");

        // method forward
        return active_manager->second.get_best_configuration(configuration_changed);
      }


      /**
       * @brief Notify the AS-RTM of the active feature cluster that the application has applied the suggested configuration
       *
       * @see Asrtm
       *
       * @details
       * This is a forward method for the Asrtm. This operation is performed only on the active state
       * of the active feature cluster.
       * Therefore, in case it is not defined the active cluster:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, it will lead to an undefined behavior
       */
      inline void configuration_applied( void )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // make sure that there is an active feature cluster
        assert(active_manager != managers.end() && "Error: attempt to notify a non-existent Asrtm");

        // method forward
        active_manager->second.configuration_applied();
      }


      /**
       * @brief Retrieves the mean value of a field of the current Operating Point of the active feature cluster
       *
       * @tparam segment The target segment of the Operating Point
       * @tparam field The index of the target field wihin the segment
       * @tparam T The type of the return value
       *
       * @return The mean value of the requested field
       *
       * @see Asrtm
       *
       * @details
       * This is a forward method for the Asrtm. This operation is performed only on the active state
       * of the active feature cluster.
       * Therefore, in case it is not defined the active cluster:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, it will lead to an undefined behavior
       */
      template< OperatingPointSegments segment, std::size_t field, class T = float >
      inline T get_mean( void ) const
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // make sure that there is an active feature cluster
        assert(active_manager != managers.end() && "Error: attempt to get the expected value from a non-existent Asrtm");

        // method forward
        return active_manager->second.template get_mean<segment,field,T>();
      }




      /******************************************************************
       *  CONSTRAINTS MANAGEMENT
       ******************************************************************/


      /**
       * @brief Add a constraint on the current active state of the active feature cluster
       *
       * @tparam segment The enumerator value of the target segment of the Operating Point
       * @tparam field_index The index of the target field within the target segment
       * @tparam sigma The confidence of the constraints, i.e. the number of times the standard deviation is taken into account
       * @tparam ConstraintGoal The type of the constraint's goal
       *
       * @param [in] goal_value The goal that represents the constraint
       * @param [in] priority The priority of the new constraint
       *
       * @see Asrtm
       *
       * @details
       * This is a forward method for the Asrtm. This operation is performed only on the active state
       * of the active feature cluster.
       * Therefore, in case it is not defined the active cluster:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, it will lead to an undefined behavior
       */
      template< OperatingPointSegments segment, std::size_t field_index, int sigma, class ConstraintGoal >
      inline void add_constraint( const ConstraintGoal& goal_value, const priority_type priority )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // loop over the available AS-RTM
        for( auto& asrtm_pair : managers )
        {
          // add a constraint
          asrtm_pair.second.template get_mean<segment,field_index,sigma,ConstraintGoal>(goal_value, priority);
        }
      }


      /**
       * @brief Remove a constraint from the current active state of the active feature cluster.
       *
       * @param [in] priority The priority of the target constraint to remove
       *
       * @see Asrtm
       *
       * @details
       * This is a forward method for the Asrtm. This operation is performed only on the active state
       * of the active feature cluster.
       * Therefore, in case it is not defined the active cluster:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, it will lead to an undefined behavior
       */
      inline void remove_constraint( const priority_type priority )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // loop over the available AS-RTM
        for( auto& asrtm_pair : managers )
        {
          // remove a constraint
          asrtm_pair.second.remove_constraint(priority);
        }
      }




      /******************************************************************
       *  RANK MANAGEMENT
       ******************************************************************/


      /**
       * @brief Set the rank of the active state, i.e. its objective function, of the active feature cluster
       *
       * @tparam objective Enumerator to tell if we want to maximize or minimize the objective function
       * @tparam composer Enumerator to select the way the objective function is composed
       * @tparam ...Fields The list of fields that compose the objective function
       *
       * @param [in] ...values The coefficients of each field that compose the objective function
       *
       * @see Asrtm
       *
       * @details
       * This is a forward method for the Asrtm. This operation is performed only on the active state
       * of the active feature cluster.
       * Therefore, in case it is not defined the active cluster:
       *  - if compiled in debug mode, it will trigger an assert and terminate the program
       *  - if compiled in release mode, it will lead to an undefined behavior
       */
      template< RankObjective objective, FieldComposer composer, class ...Fields >
      inline void set_rank( Fields ...values )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // loop over the available AS-RTM
        for( auto& asrtm_pair : managers )
        {
          // set the new rank definition
          asrtm_pair.second.template set_rank<objective,composer,Fields...>(values...);
        }
      }


    private:


      /**
       * @brief The list of the Application-Specific RunTime Managers
       */
      AsrtmContainer managers;


      /**
       * @brief The pointer to the active manger, for the given input feature
       */
      typename AsrtmContainer::iterator active_manager;

      /**
       * @brief The mutex used to enforce a consistent interal state
       */
      mutable std::mutex asrtm_mutex;

  };

}

#endif // MARGOT_DA_ASRTM_HDR
