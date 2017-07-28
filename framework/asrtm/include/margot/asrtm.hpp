/* core/asrtm
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



#ifndef MARGOT_ASRTM_ASRTM_HEADER
#define MARGOT_ASRTM_ASRTM_HEADER


#include <string>
#include <map>
#include <mutex>
#include <memory>


#include "margot/operating_point.hpp"
#include "margot/state.hpp"
#include "margot/knowledge_base.hpp"
#include "margot/rank_calculator.hpp"
#include "margot/config.hpp"
#include "margot/learning_state.hpp"
#include "margot/mab_sw_ucb.hpp"



/**
  * @brief The namespace for the mARGOt framework
  */
namespace margot
{



	/**
	 * @brief The Application-Specific Runtime Manager
	 *
	 * @details
	 * This class represents a unique interface toward the AS-RTM module.
	 * The main idea is that this class exposes the following functionalities:
	 *  - add/change/remove the active state
	 *  - add/remove Operating Points
	 *  - add/remove constraints
	 *  - generate a static goal
	 *  - retrieving the most suitable Operating Point
	 *  - define a rank function (e.g. "maximize/minimize something")
	 */
	class asrtm_t
	{

		public:


			/**
			 * @brief Default constructor
			 */
			asrtm_t( void );

			/**
			 * @brief Copy constructor
			 *
			 * @param [in] source The source asrtm
			 */
			asrtm_t( const asrtm_t& source ) = default;

			/**
			 * @brief Move constructor
			 *
			 * @param [in] source The source asrtm
			 */
			asrtm_t( asrtm_t&& source ) = default;

			/**
			 * @brief Assign operator (copy semantics)
			 *
			 * @param [in] source The source asrtm
			 */
			asrtm_t& operator=(const asrtm_t& source ) = default;

			/**
			 * @brief Assign operator (move semantics)
			 *
			 * @param [in] source The source asrtm
			 */
			asrtm_t& operator=( asrtm_t&& source ) = default;

			/**
			 * @brief Destructor
			 */
			~asrtm_t( void ) = default;









			/****************************************************
			 * State manipulation methods
			 ****************************************************/

			/**
			 * @brief Creates a new state
			 *
			 * @param [in] state_name The name of the new state
			 *
			 * @details
			 * This methods creates a new state, however it does
			 * not change the current state
			 */
			inline void add_state( const std::string state_name )
			{
				manager->add_state(state_name);
			}


			/**
			 * @brief Change the current state
			 *
			 * @param [in] new_state_name The name of the target state
			 *
			 * @details
			 * It changes the reference to the active state, thus all the
			 * operations that manipulate a state, such as adding a constraint,
			 * influence the new active state.
			 * This method issue a synch wrt the current knowledge base, however it
			 * does not change the current Operating Point used.
			 */
			inline void change_active_state( const std::string new_state_name )
			{
				manager->change_active_state(new_state_name);
			}


			/**
			 * @brief Remove a state
			 *
			 * @param [in] state_name The name of the target state
			 *
			 * @details
			 * We can remove only a state wich is not the active one
			 */
			inline void remove_state( const std::string state_name )
			{
				manager->remove_state(state_name);
			}

			/****************************************************
			 * Operating Points manipulation methods
			 ****************************************************/

			inline std::size_t get_number_operating_points( void ) const
			{
				return manager->get_number_operating_points();
			}

			/**
			 * @brief Remove a list of Operating Points
			 *
			 * @param [in] ops The list of Operating Points to remove from the knowledge
			 *
			 * @details
			 * To update a list of Operating Points, it is required to remove them first,
			 * than add the updated version.
			 * After the method is been called, the input list is not valid anymore.
			 * This method does not change the current Operating Point.
			 *
			 * @note
			 * Removing the last Operating Point actually deletes all the difined states,
			 * since the new structure of the Operating Points might be totally different
			 */
			void remove_operating_points( operating_points_t& ops )
			{
				manager->remove_operating_points(ops);
			}

			/**
			 * @brief Add a list of Operating Points
			 *
			 * @param [in] ops The list of Operating Points to add in the knowledge
			 *
			 * @details
			 * To update a list of Operating Points, it is required to remove them first,
			 * than add the updated version.
			 * After the method is been called, the input list is not valid anymore.
			 * This method does not change the current Operating Point.
			 */
			inline void add_operating_points( operating_points_t& ops  )
			{
				manager->add_operating_points(ops);
			}

			/**
			 * @brief Update the active state
			 *
			 * @details
			 * This method update the optimizaion problem according to:
			 *	- The values observed by the monitors (if any)
			 *	- The new values of the goals (if they are changed)
			 *
			 * If this method is not used, then the best Operating Point is never
			 * changed, unless the knowledge base is changed.
			 * This method actually adapt the application.
			 */
			void update( void )
			{
				manager->update();
			}

			/**
			 * @brief Find the best Operating Point
			 *
			 * @details
			 * This function actually find the best Operating Point according to the
			 * state requirements.
			 * Use the method get_best_operating_point to actually retrieve the best
			 * Operating Point.
			 */
			void find_best_operating_point( void )
			{
				manager->find_best_operating_point();
			}

			/**
			 * @brief Retrieve the best Operating Point for the application
			 *
			 * @param [out] changed [optional] Test whether the best configuration is changed
			 *
			 * @return The vector of the best configuration proposed
			 */
			inline configuration_t get_best_configuration( bool* changed = nullptr )
			{
				return manager->get_best_configuration(changed);
			}

			/**
			 * @brief The proposed configuration has been applied
			 *
			 * @details
			 * This method notify the framework that the proposed configuration is been applied
			 */
			inline void configuration_applied( void )
			{
				manager->configuration_applied();
			}

			/**
			 * @brief The proposed configuration has been rejected by the application
			 *
			 * @details
			 * This method notify the framework that the proposed configuration is been rejected
			 */
			inline void configuration_rejected( void )
			{
				manager->configuration_rejected();
			}

			/****************************************************
			 * Learning methods
			 ****************************************************/


			/**
			* @brief Define software-knobs that should be learnead with a SW UCB
			*
			* @param [in] software_knobs The list of admissible values for each software_knobs
			*
			* @param [in] window_size The size of the sliding window
			*
			* @param [in] uncertainty_coefficient A value that tunes the exploitation-exploration trade-off
			*
			* @param [in] reward_balance_coef A value to balance the reward from the state and the reward from the arm
			*
			* @details
			* This methods adds software knobs that must be learned at Run-Time.
			* The framework used to learn which is the best configuration of those software
			* knobs is a multi armed bandit, with the Sliding Window UCB flavor.
			*/
			inline void define_learning_sw_ucb_parameters( const learning_state_t::learning_configurations_t software_knobs,
			        const std::size_t window_size = 100,
			        const float uncertainty_coefficient = 0.5f,
			        const float reward_balance_coef = 1.0f )
			{
				manager->define_learning_sw_ucb_parameters(software_knobs, window_size, uncertainty_coefficient, reward_balance_coef);
			}

			/****************************************************
			 * State manipulation methods
			 ****************************************************/

			/**
			 * @brief Creates a static goal (from a parameter field)
			 *
			 * @param [in] d_fun The name of the parameter (its index)
			 * @param [in] c_fun The comparison function of the goal
			 * @param [in] value The actual numeric value of the goal
			 *
			 * @details
			 * This goal targets a parameter of the Operating Point and the observed value
			 * is the value of the parameter in the configuration used by the application
			 *
			 * @note
			 * When the asrtm is destroyed, all the goal generated are no more valid.
			 */
			goal_t create_static_goal_parameter( field_name_t d_fun, ComparisonFunction c_fun, margot_value_t value );

			/**
			 * @brief Creates a static goal (from a metric field)
			 *
			 * @param [in] d_fun The name of the metric (its index)
			 * @param [in] c_fun The comparison function of the goal
			 * @param [in] value The actual numeric value of the goal
			 *
			 * @details
			 * This goal targets a metric of the Operating Point and the observed value
			 * is the value of the metric in the configuration used by the application
			 *
			 * @note
			 * When the asrtm is destroyed, all the goal generated are no more valid.
			 */
			goal_t create_static_goal_metric( field_name_t d_fun, ComparisonFunction c_fun, margot_value_t value );

			/**
			 * @brief Add a constraint to the active state (on a parameter)
			 *
			 * @param [in] goal The goal that defines the constraint
			 * @param [in] field The field of the Operating Point object of the constraint
			 * @param [in] priority The priority of the constraint [0:max(size_t)]
			 *
			 * @details
			 * The constraint with the lower priority is the first to be relaxed, iff it is not
			 * possible to find a configuration that satisfies all the constraints.
			 * The constraint with the higher priority is the last to be relaxed, iff it is not
			 * possible to find a configuration that satisfies all the constraints.
			 * Low numerical values of the priority correspond to a logical high priority value.
			 */
			inline void add_parameter_constraint( goal_t& goal, const field_name_t field, const priority_t priority )
			{
				manager->add_parameter_constraint(goal, field, priority);
			}

			/**
			 * @brief Add a constraint to the active state (on a metric)
			 *
			 * @param [in] goal The goal that defines the constraint
			 * @param [in] field The field of the Operating Point object of the constraint
			 * @param [in] priority The priority of the constraint [0:max(size_t)]
			 *
			 * @details
			 * The constraint with the lower priority is the first to be relaxed, iff it is not
			 * possible to find a configuration that satisfies all the constraints.
			 * The constraint with the higher priority is the last to be relaxed, iff it is not
			 * possible to find a configuration that satisfies all the constraints.
			 * Low numerical values of the priority correspond to a logical high priority value.
			 */
			inline void add_metric_constraint( goal_t& goal, const field_name_t field, const priority_t priority )
			{
				manager->add_metric_constraint(goal, field, priority);
			}

			/**
			 * @brief Remove a constraint from the active state
			 *
			 * @param [in] priority The priority of the constraint
			 */
			inline void remove_constraint( const priority_t priority )
			{
				return manager->remove_constraint(priority);
			}

			/**
			* @brief Define the rank function
			*
			* @param [in] direction Specify if the user wants to maximize or minimize the rank value
			* @param [in] op_fields The list of the terms that specify the rank
			*
			* @details
			* The rank of an Operating Point, using the linear rank, defined as:
			*    rank = t1.coefficient*t1.field + t2.coefficient*t2.field + ...
			*/
			template<class ...T>
			inline void define_linear_rank(const RankObjective direction, const T... op_fields)
			{
				manager->define_linear_rank(direction, op_fields...);
			}

			/**
			 * @brief Define the rank function
			 *
			 * @param [in] direction Specify if the user wants to maximize or minimize the rank value
			 * @param [in] op_fields The list of the terms that specify the rank
			 *
			 * @details
			 * The rank of an Operating Point, using the geometric rank, defined as:
			 *    rank = t1.field^t1.coefficient * t2.field^t2.coefficient * ...
			 */
			template<class ...T>
			inline void define_geometric_rank(const RankObjective direction, const T... op_fields)
			{
				manager->define_geometric_rank(direction, op_fields...);
			}


			/****************************************************
			 * Accessory methods
			 ****************************************************/

			/**
			 * @brief Retrieve the current expected value of a metric
			 *
			 * @param [in] field_name The name (index) of the target metric
			 *
			 * @return The value of the target metric
			 */
			inline metric_t get_metric_value( const field_name_t field_name ) const
			{
				return manager->get_metric_value(field_name);
			}

			/**
			 * @brief Retrieve the current value of a parameter
			 *
			 * @param [in] field_name The name (index) of the target parameter
			 *
			 * @return The value of the target parameter
			 */
			inline parameter_t get_parameter_value( const field_name_t field_name ) const
			{
				return manager->get_parameter_value(field_name);
			}

			/**
			 * @brief Dump the state of the asrtm to the std out
			 */
			inline void dump( void ) const
			{
				return manager->dump();
			}





			/**
			 * @brief The Application-Specific Runtime Manager internal class
			 *
			 * @details
			 * This class implements the real functionality exposed by the wrapper class.
			 * The main idea is to have all the data-structure allacated in the heap, so
			 * it is possible to the the data to outlive the container variable.
			 */
			class asrtm_internal_t
			{


					/**
					 * @brief The internal state of the AS-RTM
					 *
					 * @details
					 * This enum represents the internal state of the AS-RTM, in particular
					 * it is used to track the actuation of the changes in the application.
					 */
					enum class AsrtmState : int
					{

						/**
						 * @details
						 * in this state the manager has no Operating Points, thus it is unable to
						 * provide any functionalities toward the application
						 */
						Empty,


						/**
						 * @details
						 * in this state the manager has some Operating Points, however the application
						 * has never retrieved a configuration
						 */
						Initialized,

						/**
						 * @details
						 * in this state the application is using the proposed best Operating Point
						 * suggested by the manager.
						 */
						Running,

						/**
						 * @details
						 * in this state the manager has found a better op for the application.
						 * However, the application is not aware of that yet, it will be when it
						 * gets the best op from the AS-RTM.
						 */
						NeedAdapt,


						/**
						 * @details
						 * in this state the manager is waiting for the application to actuate the
						 * proposed Operating Point. While in this state, the update and
						 * find_best_operating_point method are disabled.
						 */
						Configuring
					};


					/**
					 * @brief Container to store the observation errors
					 */
					using observation_errors_container_t = std::unordered_map<configuration_t, state_t::observation_errors_t, configuration_hash_t>;


					/**
					 * @brief The structure that represents the full state
					 *
					 * @details
					 * This struct includes a state which embeds a Design-Time knowledge of the behaviour
					 * of the application (Operating Point list) and a state that aims at learning the
					 * behaviour of the application at runtime.
					 */
					typedef struct
					{
						state_t explored_state;
#ifdef MARGOT_LEARNING_ENABLE_STATE
						learning_state_ptr_t learning_state;
						observation_errors_container_t obervation_errors;
#endif
					} asrtm_state_t;


					/**
					 * @brief The structure to store the states
					 */
					using state_map_t = std::map<std::string, asrtm_state_t>;


				public:


					/****************************************************
					 * Constructors and Deconstructors
					 ****************************************************/

					/**
					 * @brief Default constructor
					 *
					 * @details
					 * It creates a default state called "default". All the methods that
					 * affect the current state, such as adding a constraint, affect the
					 * default state, unless it is changed by the user.
					 */
					asrtm_internal_t( void );

					/**
					 * @brief Copy constructor
					 *
					 * @param [in] source The source asrtm
					 */
					asrtm_internal_t( const asrtm_internal_t& source ) = delete;

					/**
					 * @brief Move constructor
					 *
					 * @param [in] source The source asrtm
					 */
					asrtm_internal_t( asrtm_internal_t&& source ) = delete;

					/**
					 * @brief Assign operator (copy semantics)
					 *
					 * @param [in] source The source asrtm
					 */
					asrtm_internal_t& operator=(const asrtm_internal_t& source ) = delete;

					/**
					 * @brief Assign operator (move semantics)
					 *
					 * @param [in] source The source asrtm
					 */
					asrtm_internal_t& operator=( asrtm_internal_t&& source ) = delete;

					/**
					 * @brief Destructor
					 */
					~asrtm_internal_t( void ) = default;







					/****************************************************
					 * State manipulation methods
					 ****************************************************/

					/**
					 * @brief Creates a new state
					 *
					 * @param [in] state_name The name of the new state
					 *
					 * @details
					 * This methods creates a new state, however it does
					 * not change the current state
					 */
					void add_state( const std::string state_name );


					/**
					 * @brief Change the current state
					 *
					 * @param [in] new_state_name The name of the target state
					 *
					 * @details
					 * It change the reference to the active state, thus all the
					 * operations that manipulate a state, such as adding a constraint,
					 * influence the new active state.
					 * This method issue a synch wrt the current knowledge base, however it
					 * does not change the current Operating Point used.
					 */
					void change_active_state( const std::string new_state_name );

					/**
					 * @brief Remove a state
					 *
					 * @param [in] state_name The name of the target state
					 *
					 * @details
					 * We can remove only a state wich is not the active one
					 */
					void remove_state( const std::string state_name );





					/****************************************************
					 * Learning methods
					 ****************************************************/


					/**
					 * @brief Define software-knobs that should be learnead with a SW UCB
					 *
					 * @param [in] software_knobs The list of admissible values for each software_knobs
					 *
					 * @param [in] window_size The size of the sliding window
					 *
					 * @param [in] uncertainty_coefficient A value that tunes the exploitation-exploration trade-off
					 *
					 * @param [in] reward_balance_coef A value to balance the reward from the state and the reward from the arm
					 *
					 * @details
					 * This methods adds software knobs that must be learned at Run-Time.
					 * The framework used to learn which is the best configuration of those software
					 * knobs is a multi armed bandit, with the Sliding Window UCB flavor.
					 */
					void define_learning_sw_ucb_parameters( const learning_state_t::learning_configurations_t software_knobs,
					                                        const std::size_t window_size = 100,
					                                        const float uncertainty_coefficient = 0.5f,
					                                        const float reward_balance_coef = 1.0f );




					/****************************************************
					 * Operating Points manipulation methods
					 ****************************************************/


					inline std::size_t get_number_operating_points( void ) const
					{
						return knowledge.size();
					}

					/**
					 * @brief Remove a list of Operating Points
					 *
					 * @param [in] ops The list of Operating Points to remove from the knowledge
					 *
					 * @details
					 * To update a list of Operating Points, it is required to remove them first,
					 * than add the updated version.
					 * After the method is been called, the input list is not valid anymore.
					 * This method does not change the current Operating Point.
					 *
					 * @note
					 * Removing the last Operating Point actually deletes all the difined states,
					 * since the new structure of the Operating Points might be totally different
					 */
					void remove_operating_points( operating_points_t& ops );

					/**
					 * @brief Add a list of Operating Points
					 *
					 * @param [in] ops The list of Operating Points to add in the knowledge
					 *
					 * @details
					 * To update a list of Operating Points, it is required to remove them first,
					 * than add the updated version.
					 * After the method is been called, the input list is not valid anymore.
					 * This method does not change the current Operating Point.
					 */
					void add_operating_points( operating_points_t& ops  );

					/**
					 * @brief Update the active state
					 *
					 * @details
					 * This method update the optimizaion problem according to:
					 *	- The values observed by the monitors (if any)
					 *	- The new values of the goals (if they are changed)
					 *
					 * If this method is not used, then the best Operating Point is never
					 * changed, unless the knowledge base is changed.
					 * This method actually adapt the application.
					 */
					void update( void );

					/**
					 * @brief Find the best Operating Point
					 *
					 * @details
					 * This function actually find the best Operating Point according to the
					 * state requirements.
					 * Use the method get_best_operating_point to actually retrieve the best
					 * Operating Point.
					 */
					void find_best_operating_point( void );

					/**
					 * @brief Retrieve the best Operating Point for the application
					 *
					 * @param [out] changed [optional] Test whether the best configuration is changed
					 *
					 * @return The vector of the best configuration proposed
					 */
					configuration_t get_best_configuration( bool* changed = nullptr );

					/**
					 * @brief The proposed configuration has been applied
					 *
					 * @details
					 * This method notify the framework that the proposed configuration is been applied
					 */
					void configuration_applied( void );

					/**
					 * @brief The proposed configuration has been rejected by the application
					 *
					 * @details
					 * This method notify the framework that the proposed configuration is been rejected
					 */
					void configuration_rejected( void );













					/****************************************************
					 * State manipulation methods
					 ****************************************************/

					/**
					 * @brief Add a constraint to the active state (on a parameter)
					 *
					 * @param [in] goal The goal that defines the constraint
					 * @param [in] field The field of the Operating Point object of the constraint
					 * @param [in] priority The priority of the constraint [0:max(size_t)]
					 *
					 * @details
					 * The constraint with the lower priority is the first to be relaxed, iff it is not
					 * possible to find a configuration that satisfies all the constraints.
					 * The constraint with the higher priority is the last to be relaxed, iff it is not
					 * possible to find a configuration that satisfies all the constraints.
					 * Low numerical values of the priority correspond to a logical high priority value.
					 */
					void add_parameter_constraint( goal_t& goal, const field_name_t field, const priority_t priority );

					/**
					 * @brief Add a constraint to the active state (on a metric)
					 *
					 * @param [in] goal The goal that defines the constraint
					 * @param [in] field The field of the Operating Point object of the constraint
					 * @param [in] priority The priority of the constraint [0:max(size_t)]
					 *
					 * @details
					 * The constraint with the lower priority is the first to be relaxed, iff it is not
					 * possible to find a configuration that satisfies all the constraints.
					 * The constraint with the higher priority is the last to be relaxed, iff it is not
					 * possible to find a configuration that satisfies all the constraints.
					 * Low numerical values of the priority correspond to a logical high priority value.
					 */
					void add_metric_constraint( goal_t& goal, const field_name_t field, const priority_t priority );

					/**
					 * @brief Remove a constraint from the active state
					 *
					 * @param [in] priority The priority of the constraint
					 */
					void remove_constraint( const priority_t priority );

					/**
					* @brief Define the rank function
					*
					* @param [in] direction Specify if the user wants to maximize or minimize the rank value
					* @param [in] op_fields The list of the terms that specify the rank
					*
					* @details
					* The rank of an Operating Point, using the linear rank, defined as:
					*    rank = t1.coefficient*t1.field + t2.coefficient*t2.field + ...
					*/
					template<class ...T>
					inline void define_linear_rank(const RankObjective direction, const T... op_fields)
					{
						std::lock_guard<std::mutex> lock(asrtm_mutex);

						if ( internal_state != AsrtmState::Empty )
						{
							current_state->second.explored_state.define_linear_rank(direction, op_fields...);
							structure_changed = true;
						}
					}

					/**
					 * @brief Define the rank function
					 *
					 * @param [in] direction Specify if the user wants to maximize or minimize the rank value
					 * @param [in] op_fields The list of the terms that specify the rank
					 *
					 * @details
					 * The rank of an Operating Point, using the geometric rank, defined as:
					 *    rank = t1.field^t1.coefficient * t2.field^t2.coefficient * ...
					 */
					template<class ...T>
					inline void define_geometric_rank(const RankObjective direction, const T... op_fields)
					{
						std::lock_guard<std::mutex> lock(asrtm_mutex);

						if ( internal_state != AsrtmState::Empty )
						{
							current_state->second.explored_state.define_geometric_rank(direction, op_fields...);
							structure_changed = true;
						}
					}










					/****************************************************
					 * Accessory methods
					 ****************************************************/

					/**
					 * @brief Retrieve the current expected value of a metric
					 *
					 * @param [in] field_name The name (index) of the target metric
					 *
					 * @return The value of the target metric
					 */
					inline metric_t get_metric_value( const field_name_t field_name ) const
					{
#ifdef MARGOT_LEARNING_ENABLE_STATE
						return internal_state != AsrtmState::Empty ?
						       knowledge.get_operating_point(configuration_t(actual_configuration.begin(), actual_configuration.begin() + explored_portion_configuration_size)).second[field_name] : static_cast<metric_t>(0);
#else
						return internal_state != AsrtmState::Empty ? knowledge.get_operating_point(actual_configuration).second[field_name] : static_cast<metric_t>(0);
#endif
					}

					/**
					 * @brief Retrieve the current value of a parameter
					 *
					 * @param [in] field_name The name (index) of the target parameter
					 *
					 * @return The value of the target parameter
					 */
					inline parameter_t get_parameter_value( const field_name_t field_name ) const
					{
						return internal_state != AsrtmState::Empty ? actual_configuration[field_name] : static_cast<parameter_t>(0);
					}

					/**
					 * @brief Dump the state of the asrtm to the std out
					 */
					void dump( void ) const;


				private:

					/**
					 * @brief lock free, state free method to find the best configuration
					 *
					 * @param [in] state_changed If false, it means that the explored state will select the same configuration
					 */
					void find_best_configuration_internal( const bool state_changed );

					/**
					 * @brief The knoledge used by the manager
					 */
					knowledge_base_t knowledge;

					/**
					 * @brief All the know states
					 */
					state_map_t states;

					/**
					 * @brief a reference to the current state
					 */
					state_map_t::iterator current_state;

					/**
					 * @brief a copy of the best configuration found
					 */
					configuration_t proposed_best_configuration;

					/**
					 * @brief a copy of the actual configuration used by the application
					 */
					configuration_t actual_configuration;

					/**
					 * @brief states if we have removed the current Operating Point
					 */
					bool removed_current_operating_point;

#ifdef MARGOT_LEARNING_ENABLE_STATE
					/**
					 * @brief this is the number of paramters explored during the DSE
					 */
					std::size_t explored_portion_configuration_size;
#endif // MARGOT_LEARNING_ENABLE_STATE

					/**
					 * @brief the state of the manager
					 */
					AsrtmState internal_state;

					/**
					 * @brief a flag that states if there is a change in the structure of the problem
					 */
					bool structure_changed;

					/**
					 * @brief a flag that states if the update function has changed the state
					 */
					bool state_updated;

					/**
					 * @brief The mutex used to synchronize all the operations on the AS-RTM
					 */
					mutable std::mutex asrtm_mutex;

			};


			/**
			 * @brief Typedef to the pointer to the internal representation
			 */
			using asrtm_internal_ptr_t = std::shared_ptr<asrtm_internal_t>;



			/**
			 * @brief Retrieve the a copy of the internal structure
			 */
			inline asrtm_internal_ptr_t get_manager_ptr( void ) const
			{
				return manager;
			}

		private:


			/**
			 * @brief The pointer to the data structure
			 */
			asrtm_internal_ptr_t manager;

	};

}




#endif // MARGOT_ASRTM_ASRTM_HEADER
