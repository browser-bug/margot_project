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
#include <deque>
#include <cstddef>
#include <cassert>
#include <mutex>
#include <functional>
#include <iostream>
#include <array>
#include <utility>
#include <type_traits>

#include "margot/operating_point.hpp"
#include "margot/asrtm.hpp"
#include "margot/debug.hpp"
#include "margot/data_features.hpp"

#ifdef MARGOT_WITH_AGORA
  #include <thread>
  #include <sstream>
  #include <chrono>
  #include <ctime>
  #include <map>
  #include <algorithm>
  #include "agora/virtual_channel.hpp"
#endif // MARGOT_WITH_AGORA

namespace margot
{

  /**
   * @brief The Data Aware Application-Specific RunTime Manger implementation
   *
   * @tparam Asrtm The type which defines the underlying Application-Specific RunTime managers
   * @tparam T The type of the elements stored in the data features
   * @tparam distance_type The enumerator that defines how distance between features are computed
   * @tparam cfs.. The type of comparator which define a vald data feature candidate
   *
   * @see Asrtm
   *
   * @details
   * This class represent the interface of the mARGOt dynamic autotuner toward the appplication, if we want to
   * take in consideration heterogeneous input, without any assumption on the order of the input.
   * The idea of this class is to allocate a separate Asrtm for each cluster of input, represented by the Feature
   * template parameter, i.e. an array of numeric values.
   * At run-time the application set the correct data features for the actual input, and this class will use that value
   * for selecting the most suitable configuration.
   * To define the "closest" datafeature, we implemented the concept in two stage, in the first stage we compara the
   * validity of the data features, using the cfs template parameters, then we compute the distance from the requested
   * key and we choose the closest one. If there are more than one feature cluster at the same distance of the requested
   * one, it will be chosen the one that happens to be inserted first.
   * All the methods that defines the optimization problem, e.g. adding a constraint, will apply to all the Input
   * features. The other methods affects only the "active" Asrtm.
   *
   * However, all the methods are mutex protected, to enforce a consistent internal state.
   *
   * @note
   * The type T must be numeric and there must be at least one data feature.
   * The number of FeatureComparison must be equal to the number of data features. The value of the data feature
   * comparator must be read as data_feature_cluster_x[i-th] must be <comparison> than target_feature[i-th]
   */
  template< class Asrtm, typename T, FeatureDistanceType distance_type, FeatureComparison... cfs >
  class DataAwareAsrtm
  {


      /**
       * @brief Explicit definition of this Data-Aware Asrtm type
       */
      using type = DataAwareAsrtm<Asrtm, T, distance_type, cfs...>;



      // statically check the template arguments
      static_assert(traits::is_operating_point<typename Asrtm::operating_point_type>::value,
                    "Error: the Data-Aware Application-Specific RunTime Manager handles object with is_operating_point trait");

      static_assert(sizeof...(cfs) > 0, "Error: there must be at least one Data Feature");

      static_assert( std::is_arithmetic<T>::value, "Error: the data features value type must be numerical");


    public:


      /**
       * @brief Explicit definition of the Operating points
       */
      using OperatingPoint = typename Asrtm::operating_point_type;


      /**
       * @brief Explicit definition of the priority type
       */
      using priority_type = typename Asrtm::priority_type_type;


      /**
       * @brief Explicit definition of the state id type
       */
      using state_id_type = typename Asrtm::state_id_type_type;


      /**
       * @brief Explicit definition of the Data Features
       */
      using Feature = std::array<T, sizeof...(cfs)>;


      /**
       * @brief Explicit definition to an Operating Point pointer
       */
      using OperatingPointPtr = typename Knowledge<OperatingPoint>::OperatingPointPtr;


      /**
       * @brief Explicit definition of the stored element in the container
       */
      using AsrtmElement = std::pair< Feature, Asrtm >;


      /**
       * @brief The definition of the container of the Asrtm
       */
      using AsrtmContainer = std::deque< AsrtmElement >;


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
       * other operation, such as defining a rank or adding a constraint.
       */
      DataAwareAsrtm( void ): get_closest()
      {
        // enforce the actual state of the AS-RTM
        active_manager = managers.end();
      }


#ifdef MARGOT_WITH_AGORA

      /**
       * @brief Default destructor
       *
       * @details
       * When the Data-Aware AS-RTM interacts directly with the remote
       * application handler, it uses a separate thread for the synchronization.
       * This method ensures that when the manager is destroyed, it gracefully
       * ends the connecction with MQTT and it joins the thread.
       */
      ~DataAwareAsrtm( void )
      {
        if (local_handler.joinable())
        {
          remote.destroy_channel();
          local_handler.join();
        }
      }

#endif // MARGOT_WITH_AGORA

      /******************************************************************
       *  METHODS TO MANAGE THE FEATURE CLUSTERS
       ******************************************************************/


      /**
       * @brief Create a new data feature cluster using the target key
       *
       * @param [in] key The std::array of features that defines the new cluster
       *
       * @details
       * The new Application-Specific RunTime manager is either a flat new object, if
       * it is the first manager created. Otherwise, this method uses the first
       * asrtm to get all the states and runtime information (perform a pseudo-copy).
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
        if (!managers.empty())
        {
          // we need to get a copy of the current key
          const auto key_actual_cluster = active_manager->first;

          // emplace the new asrtm (if it's actually new)
          managers.emplace_back(key, managers.begin()->second.create_sibling());

          // reset the iterator to the current manager
          const auto end_iterator = managers.end();

          for ( active_manager = managers.begin(); active_manager != end_iterator; ++active_manager )
          {
            if (active_manager->first == key_actual_cluster)
            {
              break;
            }
          }
        }
        else
        {
          // it's the first cluster
          managers.emplace_back(key, Asrtm{});
          active_manager = managers.begin();
        }
      }


      /**
       * @brief Change the active feature cluster
       *
       * @param [in] key The data feature of the actual input
       *
       * @details
       * This method tries to select the correct feature cluster given the key in
       * input, finding the closest one from the valid cluster.
       * The key of a cluster is valid if it respect the constraint specified as template
       * parameter "cfs" of this class. The order of the template parameter must match the
       * index of the data feature. And the comparison function stands for "the key of the
       * evaluated feature cluster must be <comparator> than the one in input".
       * The comparison value "DONT_CARE", automatically validates the target field.
       */
      void select_feature_cluster( const Feature key )
      {
        // make sure that no one else is going to change the container
        std::lock_guard<std::mutex> lock(asrtm_mutex);

        // take a copy of the previous active state
        const auto previous_active_manager = active_manager;

        // assume that the first manager is the correct one
        active_manager = managers.begin();

        // loop to see if it is the actual best manager
        const auto final_iterator = managers.end();

        for ( auto it = std::next(active_manager); it != final_iterator; ++it )
        {
          active_manager = get_closest(key, active_manager, it);
        }

        // reset the information about the current Operating Point (if we change the active manager)
        if (previous_active_manager != active_manager)
        {
          active_manager->second.restore_from_data_feature_switch();
        }
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

        // since erasing an element invalidates all the iterator, use temporaney
        // the active_manager iterator as the end iterator, then we will restore
        // it's actual value, but in this way we are safe in case is not yet
        // defined the actual manager
        const auto previous_active = active_manager;
        active_manager = managers.end();

        // store the key of the previous cluster (if any)
        const auto key_active = active_manager != active_manager ? active_manager->first : key;

        // remove the target asrtm
        for ( auto it = managers.begin(); it != active_manager; ++it )
        {
          if (it->first == key)
          {
            assert(previous_active != it && "Error: attempting to delete the active cluster");
            managers.erase(it);
            break;
          }
        }

        // find again the active manager (if any)
        for ( auto it = managers.begin(); it != active_manager; ++it)
        {
          if (it->first == key_active)
          {
            active_manager = it;
            break;
          }
        }
      }


      /**
       * @brief Retrieve the value of a field of the active data feature
       *
       * @tparam index The index of the target field of the data feature
       *
       * @return the numeric value of the target field
       *
       * @details
       * If it is not set any active manager will be returned a default
       * value, according to the type T, usually zero.
       */
      template< std::size_t index >
      inline T get_selected_feature( void ) const
      {
        static_assert( index < sizeof...(cfs), "Error: attempt to access an out of bound feature");
        std::lock_guard<std::mutex> lock(asrtm_mutex);
        return active_manager != managers.end() ? std::get<index>(active_manager->first) : T{};
      }




      /******************************************************************
       *  FORWARD METHODS TO MANAGE THE APPLICATION KNOWLEDGE
       ******************************************************************/


      /**
       * @brief Add Operating Points to the active AS-RTM
       *
       * @tparam Y An object which implements the STL Container concept and stores Operating Points
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
      template< class Y >
      inline std::size_t add_operating_points( const Y& op_list )
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
       * @tparam Y An object which implements the STL Container concept and stores Operating Points or configurations
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
      template< class Y >
      inline std::size_t remove_operating_points( const Y& configuration_list )
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
       * @return True, if the knowledge base is empty or there is no active manager
       *
       * @see Asrtm
       *
       * @details
       * It is forbidden to check if there are Operating Points in a non-defined active cluster.
       * For this reason this method return true also in case that there are no feature cluster
       * or the active feature cluster is undefined
       */
      inline bool is_application_knowledge_empty( void ) const
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // method forward
        return active_manager != managers.end() ? active_manager->second.is_application_knowledge_empty() : true;
      }


      /**
       * @brief Test whether the status of the current manager is in design space exploration
       *
       * @return True, if we are performing a design space exploration
       */
      inline bool in_design_space_exploration( void ) const
      {
        std::lock_guard< std::mutex > lock(asrtm_mutex);
        return active_manager != managers.end() ? active_manager->second.in_design_space_exploration() : true;
      }


      /**
       * @brief Test whether the current manager has the model
       *
       * @return True, if we have the model
       */
      inline bool has_model( void ) const
      {
        std::lock_guard< std::mutex > lock(asrtm_mutex);
        return active_manager != managers.end() ? active_manager->second.has_model() : false;
      }



      /******************************************************************
       *  METHODS TO MANAGE THE BEHOLDER'S COMMANDS
       ******************************************************************/


      /**
       * @brief Test whether the metrics are to be enabled (indirectly controlled by the beholder)
       *
       * @return True, if the (potentially disabled) metrics are to be computed (e.g. error)
       */
      inline bool are_metrics_on ( void ) const
      {
        std::lock_guard< std::mutex > lock(asrtm_mutex);
        return active_manager != managers.end() ? active_manager->second.are_metrics_on() : false;
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
        for ( auto& asrtm_pair : managers )
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
        for ( auto& asrtm_pair : managers )
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
        for ( auto& asrtm_pair : managers )
        {
          // switch to a new state
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
       * @tparam Y The type of elements stored in the monitor
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
                class Y,
                typename statistical_t >
      inline void add_runtime_knowledge( const Monitor<Y, statistical_t>& monitor )
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // loop over the available AS-RTM
        for ( auto& asrtm_pair : managers )
        {
          // create a new state
          asrtm_pair.second.template add_runtime_knowledge<target_segment, target_field_index, inertia, Y, statistical_t>(monitor);
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
        for ( auto& asrtm_pair : managers )
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
       * @tparam Y The type of the return value
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
      template< OperatingPointSegments segment, std::size_t field, class Y = float >
      inline Y get_mean( void ) const
      {
        // lock the manger mutex, to ensure a consistent global state
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // make sure that there is an active feature cluster
        assert(active_manager != managers.end() && "Error: attempt to get the expected value from a non-existent Asrtm");

        // method forward
        return active_manager->second.template get_mean<segment, field, T>();
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
        for ( auto& asrtm_pair : managers )
        {
          // add a constraint
          asrtm_pair.second.template add_constraint<segment, field_index, sigma, ConstraintGoal>(goal_value, priority);
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
        for ( auto& asrtm_pair : managers )
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
        for ( auto& asrtm_pair : managers )
        {
          // set the new rank definition
          asrtm_pair.second.template set_rank<objective, composer, Fields...>(values...);
        }
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
       * @see Asrtm
       */
      inline void send_observation( const std::string& measures )
      {
        std::lock_guard< std::mutex > lock(asrtm_mutex);

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
       * @see Asrtm
       */
      template< class OpConverter >
      void start_support_thread( const std::string& application, const std::string& broker_url, const std::string& username, const std::string& password, const int qos_level,
                                 const std::string& description, const std::string& broker_ca, const std::string& client_cert, const std::string& client_key  )
      {
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        // get the application name
        application_name = application;

        // disable the agora logging
        agora::my_agora_logger.set_filter_at(agora::LogLevel::DISABLED);

        // set the Design Space Exploration state in the cluster managers
        for ( auto& asrtm_pair : managers )
        {
          asrtm_pair.second.set_autotuner_in_dse();
        }

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
       * @brief This method prints on the standard output the state of the da_asrtm
       */
      void dump( void ) const;


    private:




      /******************************************************************
       *  AGORA LOCAL APPLICATION HANDLER PRIVATE METHODS
       ******************************************************************/


#ifdef MARGOT_WITH_AGORA


      /**
       * @brief
       * Enables the metrics to be computed (indirectly controlled by the beholder)
       *
       * @details
       * It calls the corresponding asrtm function on the AS-RTM of all the feature clusters
       * independently of which is the active_manager which is not modified
       */
      inline void set_metrics_on ( void )
      {
        std::lock_guard< std::mutex > lock(asrtm_mutex);

        for ( auto& asrtm_pair : managers )
        {
          asrtm_pair.second.set_metrics_on();
        }
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

        // register to the application topic to receive the beholder's commands
        remote.subscribe("margot/" + application_name + "/commands");

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
            // build the op
            const auto op = get_op(new_incoming_message.payload);

            // lock the asrtm data structure
            std::lock_guard<std::mutex> lock(asrtm_mutex);

            // make sure that there is an active feature cluster
            assert(!managers.empty() && "Error: unable to explore a configuration without AS-RTM");

            // set the knowledge base for all the data features
            for ( auto& asrtm_pair : managers )
            {
              asrtm_pair.second.set_sinlgle_point(op);
            }
          }
          else  if (message_topic.compare("/info") == 0)   // handle the info message
          {
            remote.send_message({{"margot/" + application_name + "/info"}, application_description});
          }
          else if (message_topic.compare("/model") == 0) // handle the final model coming from the server
          {
            // prepare the data structures to have a model
            std::map< std::string, std::vector< OperatingPoint > > model;
            std::stringstream model_stream(new_incoming_message.payload);
            constexpr char line_delimiter = '@';
            std::string op_string;

            // parse all the Operating Point from the model
            while (std::getline(model_stream, op_string, line_delimiter))
            {
              // parse the data structure
              std::string knobs;
              std::string features;
              std::string metrics;
              std::stringstream op_stream(op_string);
              op_stream >> knobs;
              op_stream >> features;
              op_stream >> metrics;

              // find the related elements in the map
              auto map_it = model.find(features);

              if (map_it == model.end())
              {
                // create the model
                const auto return_pair = model.emplace(features, std::vector< OperatingPoint > {});
                map_it = return_pair.first;
              }

              // insert the related op
              map_it->second.emplace_back(get_op(knobs, metrics));
            }

            // now we need to modify the da-asrtm structure
            std::lock_guard<std::mutex> lock(asrtm_mutex);

            // make sure that there is an active feature cluster
            assert(!managers.empty() && "Error: unable to set the model without AS-RTM");

            // get a clone of the manager
            auto reference_manager = managers.begin()->second.create_sibling();

            // clear all the previous managers
            managers.clear();

            // set the new nowledge
            for ( auto&& model_pair : model )
            {
              Feature this_model_feature;

              // parse the feature of this string
              std::stringstream feature_stream(model_pair.first);
              std::string feature_element;
              std::size_t counter = 0;

              while (std::getline(feature_stream, feature_element, ','))
              {
                std::istringstream( feature_element ) >> this_model_feature[counter++];
              }

              // emplace the new element
              managers.emplace_back(this_model_feature, reference_manager.create_sibling());

              // set the knowledge base
              managers.back().second.set_model(std::move(model_pair.second));
            }

            // set the current manager as the first one
            active_manager = managers.begin();

          }
          else if (message_topic.compare("/welcome") == 0) // handle the case where a new agora handler appears
          {
            // send a welcome message to restore the communication
            remote.send_message({{"margot/" + application_name + "/welcome"}, my_client_id});
          }
          // handle the messages coming from the beholder
          else if (message_topic.compare("/commands") == 0)
          {
            // enables all the metrics (which could be potentially disabled)
            if (new_incoming_message.payload.compare("metrics_on") == 0)
            {
              set_metrics_on();
            }
          }
        }
      }

#endif // MARGOT_WITH_AGORA


      /**
       * @brief The list of the Application-Specific RunTime Managers
       */
      AsrtmContainer managers;


      /**
       * @brief The object used to select the closest AS-RTM
       */
      const data_feature_selector<Feature, typename AsrtmContainer::iterator, distance_type, cfs...> get_closest;


      /**
       * @brief The pointer to the active manger, for the given input feature
       */
      typename AsrtmContainer::iterator active_manager;

      /**
       * @brief The mutex used to enforce a consistent interal state
       */
      mutable std::mutex asrtm_mutex;


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



  template< class Asrtm, typename T, FeatureDistanceType distance_type, FeatureComparison... cfs >
  void DataAwareAsrtm<Asrtm, T, distance_type, cfs... >::dump( void ) const
  {
    // print out loud the header
    print_header();

    // macro to print the data feature value
    const auto feature2string = [] ( const Feature & f )
    {
      std::string result = "[";
      std::size_t counter = 0;

      for ( const auto value : f )
      {
        result += std::string(" ") + std::to_string(value);

        if (counter > 0)
        {
          result += ",";
        }

        ++counter;
      }

      result += " ]";
      return result;
    };

    // print information regarding the outer loop
    std::cout << "# Data-Aware Application-Specific RunTime Manager status dump" << std::endl;
    std::cout << "#" << std::endl;
    std::cout << "# Number of data feature cluster: " << managers.size() << std::endl;

    if ( active_manager != managers.end() )
    {
      // loop some statistics about the cluster
      std::cout << "# Active feature cluster address: " << &active_manager->second << std::endl;
      std::cout << "# Active feature cluster key value: " << feature2string(active_manager->first) << std::endl;
    }
    else
    {
      std::cout << "# Active feature cluster address: N/A" << std::endl;
      std::cout << "# Active feature cluster key value: N/A" << std::endl;
    }

    // print each active state
    const auto key = active_manager->first;

    // print the details about each feature cluster
    for ( const auto& manager_pair : managers )
    {
      // print the header for the data cluster
      std::string active_feature = manager_pair.first == key ? " <---- CURRENT CLUSTER " : "";
      std::cout << "#" << std::endl;
      std::cout << "# ///////////////////////////////////////////////////////////////////" << std::endl;
      std::cout << "# //       FEATURE CLUSTER KEY: " << feature2string(manager_pair.first) << active_feature << std::endl;

      // print the detail about the data cluster
      manager_pair.second.dump(false);

      // print the trailer for the data cluster
      std::cout << "#" << std::endl;
    }





    // print the trailer
    print_trailer();

  }

}

#endif // MARGOT_DA_ASRTM_HDR
