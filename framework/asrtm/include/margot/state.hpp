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


#ifndef MARGOT_ASRTM_STATE_HEADER
#define MARGOT_ASRTM_STATE_HEADER

#include <memory>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <set>
#include <functional>
#include <cstdint>
#include <map>


#include "margot/config.hpp"
#include "margot/operating_point.hpp"
#include "margot/goal.hpp"
#include "margot/view.hpp"
#include "margot/knowledge_base.hpp"
#include "margot/rank_calculator.hpp"


/**
 * @brief The namespace for the mARGOt framework
 */
namespace margot
{


	/**
	 * @brief Manages a state of the AS-RTM
	 *
	 * @details
	 * The state represents the application requiments of the application.
	 * The requirements are defined by the a list of constraints and the rank
	 * definition.
	 * The objective of a state is to:
	 *   - obtain the most suitable configuration for the application
	 *   - synchonize its internal version wrt the knowledge stored in the manager
	 *   - expose manipulation methods (i.e. defining the rank, add a goal, etc. etc.)
	 */
	class state_t
	{

		public:

			/**
			* @brief Define the list of the observation error
			*/
			using observation_errors_t = std::unordered_map<field_name_t, margot_value_t>;



			/****************************************************
			 * State constants for the reward computation
			 ****************************************************/

			/**
			 * @brief Is the balance coeffient between the rank factor and the constraints factor
			 *
			 * @details
			 * This number should vary between [0,1] and states how important is the constraints
			 * factor with respect to the rank factor.
			 * Values close to 1 give more importance to the constraint factor.
			 * Values close to 0 give more importance to the rank factor.
			 */
			static constexpr float reward_balance_coefficiet = 0.5f;

			/**
			 * @brief The maximum value of the reward
			 */
			static constexpr float reward_max_value = 1000.0f;



			/****************************************************
			 * Constructors and destructor of the state
			 ****************************************************/

			/**
			 * @brief Default constructor
			 */
			state_t( void ): state(new state_internal_t()) {}

			/**
			 * @brief Copy constructor
			 */
			state_t( const state_t& other) = default;

			/**
			 * @brief Move constructor
			 */
			state_t( state_t&& other) = default;

			/**
			 * @brief Assignment operator (copy semantic)
			 */
			state_t& operator=(const state_t& other) = default;

			/**
			 * @brief Assignment operator (move semantic)
			 */
			state_t& operator=(state_t&& other) = default;

			/**
			 * @brief Deconstructor
			 */
			~state_t( void ) = default;




			/****************************************************
			 * Rank definition methods
			 ****************************************************/


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
				state->define_linear_rank(direction, op_fields...);
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
				state->define_geometric_rank(direction, op_fields...);
			}



			/**
			 * @brief Compute the reward of a configuration
			 *
			 * @param [in] configuration The evaluated configuration
			 *
			 * @result The reward [0,1], higher values are better
			 */
			inline float get_reward( const configuration_t& configuration)
			{
				return state->get_reward(std::move(configuration));
			}



			/****************************************************
			 * Operating Points manipulation methods
			 ****************************************************/


			/**
			 * @brief Get the knowledge from a knowledge base
			 *
			 * @param [in] kb The source knowledge base
			 *
			 * @details
			 * This method automatically issue a synch operation
			 * wrt the new knowledge_base.
			 */
			inline void set_knowledge_base( const knowledge_base_t& kb )
			{
				state->set_knowledge_base(kb);
			}

			/**
			 * @brief Add the Operating Points to the state
			 *
			 * @param [in] ops The list of Operating Points to be added
			 *
			 * @details
			 * This method is meant to be used when another object updates the global
			 * list of Operating Points.
			 * Usually it is called only if this state is the active one, since it
			 * requires to store the Operating Points that are introduced in the update.
			 *
			 * @note
			 * After calling this method the input parameter is not valid anymore
			 */
			inline void add_operating_points( operating_points_t& ops )
			{
				state->add_operating_points(ops);
			}


			/**
			 * @brief Removes the Operating Points from the state
			 *
			 * @param [in] ops The list of Operating Points to be removed
			 *
			 * @details
			 * This method is meant to be used when another object updates the global
			 * list of Operating Points.
			 * Usually it is called only if this state is the active one, since it
			 * requires to store the Operating Points that are removed in the update.
			 *
			 * @note
			 * After calling this method the input parameter is not valid anymore
			 */
			inline void remove_operating_points( operating_points_t& ops )
			{
				state->remove_operating_points(ops);
			}


			/**
			 * @brief Synchronize the state with knowledge base
			 *
			 * @details
			 * The synchronization happens only if the knowledge base has been updated
			 * when another state was active.
			 */
			inline void synch( void )
			{
				state->synch();
			}


			/**
			 * @brief Retrieve the best configuration
			 *
			 * @return A copy of the most suitable configuration
			 *
			 * @details
			 * The algorithm used is very simple:
			 *  - select the first configuration in the set (highest rank)
			 *  - if not available loop over the constraints from the bottom
			 *     - select the closer one between the configurations in the blocked_ops
			 */
			inline configuration_t get_best_configuration( void ) const
			{
				return state->get_best_configuration();
			}



			/**
			 * @brief Update the constraints according to the current situation
			 *
			 * @param [in] current_configuration The used configuration
			 *
			 * @param [in] without_monitor If true, the update does not consider the informations from the monitors
			 *
			 * @return True, if the internal structure has been updated
			 *
			 * @details
			 * This method updates the internal structures according to:
			 *   - the observed situation by the monitor (if any)
			 *   - changes in the goal value
			 */
			inline bool update( const configuration_t& current_configuration, const bool without_monitor = false )
			{
				return state->update(current_configuration, without_monitor);
			}




			/****************************************************
			 * Constraints manipulation methods
			 ****************************************************/



			/**
			 * @brief Add a constraint on a parameter
			 *
			 * @param [in] field The name of the target field of the Operating Point
			 * @param [in] goal The reference to the target goal
			 * @param [in] priority The priority of the constraint
			 *
			 * @details
			 * The priority is used as an unique identifier of the constraint. Moreover it is used to
			 * sort the constraint, where to priority is expressed in a decreasing order.
			 * If two constraints are created with the same priority, the std::map semantics will apply.
			 */
			inline void add_parameter_constraint(const field_name_t field, goal_t& goal, const priority_t priority)
			{
				state->add_parameter_constraint(field, goal, priority);
			}




			/**
			 * @brief Add a constraint on a metric
			 *
			 * @param [in] field The name of the target field of the Operating Point
			 * @param [in] goal The reference to the target goal
			 * @param [in] priority The priority of the constraint
			 *
			 * @details
			 * The priority is used as an unique identifier of the constraint. Moreover it is used to
			 * sort the constraint, where to priority is expressed in a decreasing order.
			 * If two constraints are created with the same priority, the std::map semantics will apply.
			 */
			inline void add_metric_constraint(const field_name_t field, goal_t& goal, const priority_t priority)
			{
				state->add_metric_constraint(field, goal, priority);
			}



			/**
			 * @brief Remove a constraint
			 *
			 * @param [in] priority The priority of the target constraint
			 */
			inline void remove_constraint(const priority_t priority)
			{
				state->remove_constraint(priority);
			}





			/****************************************************
			 * Utility methods
			 ****************************************************/

			/**
			 * @brief Clear the observation window
			 *
			 * @details
			 * This method clears the observation window of all the
			 * constraints on a metric observed a run-time
			 */
			inline void clear_monitors( void )
			{
				state->clear_monitors();
			}


			/**
			 * @brief Test whether the effects of the configuration are observed
			 *
			 * @return True if all the monitors related to the constraints are full
			 */
			inline bool is_observable( void ) const
			{
				return state->is_observable();
			}

			/**
			 * @brief Test whether the effects of the configuration are observed at all
			 *
			 * @return True if all the monitors related to the constraints are empty
			 */
			inline bool is_not_observable( void ) const
			{
				return state->is_observable();
			}


			/**
			 * @brief Retrieve the vector of the observation errors
			 *
			 * @return a vector with observation errors
			 */
			inline observation_errors_t get_observation_errors( void ) const
			{
				return state->get_observation_errors();
			}


			/**
			 * @brief Set the vector of the observation errors
			 *
			 * @param [in] new_errors The vectors of new observation errors
			 */
			inline void set_observation_errors(observation_errors_t new_errors)
			{
				return state->set_observation_errors(std::move(new_errors));
			}






			/****************************************************
			 * Debug method
			 ****************************************************/

			/**
			 * @brief Dump the state of the constraint to the std out
			 */
			inline void dump( void ) const
			{
				state->dump();
			}


			/**
			 * @brief Check if the state is self coherent
			 *
			 * @details:
			 * If the state is not sel-coherent throws a runtime_error exception
			 */
			inline void sanity_checks( void ) const
			{
				state->sanity_checks();
			}












			/**
			 * @brief Internal class that Manages a state of the AS-RTM
			 *
			 * @details
			 * The state represents the application requiments of the application.
			 * The requirements are defined by the a list of constraints and the rank
			 * definition.
			 * The objective of a state is to:
			 *   - obtain the most suitable configuration for the application
			 *   - synchonize its internal version wrt the knowledge stored in the manager
			 *   - expose manipulation methods (i.e. defining the rank, add a goal, etc. etc.)
			 */
			class state_internal_t
			{
					/**
					 * @brief The definition of a constraint
					 *
					 * @details
					 * The definition of a constraint is composed by:
					 *   - the associated goal
					 *   - the look-up table for the configurations
					 *   - a view on the target metric
					 *   - the previous effective goal value
					 *   - the priority of the constraint
					 */
					typedef struct
					{
						goal_t::target_t_ptr goal;
						lookup_table_t blocked_ops;
						view_t_ptr metric_view;
						margot_value_t previous_goal_value;
						margot_value_t previous_observation_error;
					} constraint_t;


					/**
					 * @brief define a list of constraint
					 */
					using constraints_t = std::map< priority_t, constraint_t >;



				public:

					/**
					 * @brief The map that associate a rank value to a configuration
					 */
					using ranks_t = std::unordered_map< configuration_t, rank_t, configuration_hash_t >;


					/**
					 * @brief Comparator functor for the set of valid Operating Points
					 *
					 * @param [in] lhs The left hand side operator
					 * @param [in] rhs The right hand side operator
					 *
					 * @details
					 * This functor it is required since the information used to compare two configuration
					 * is outside the configuration, in particular it is in the state variable
					 */
					struct rank_comparator_t
					{
						public:
							bool operator()( const configuration_t& lhs, const configuration_t& rhs ) const
							{
								const rank_t value1 = ref_to_rank_values->at(lhs);
								const rank_t value2 = ref_to_rank_values->at(rhs);

								if ( value1 == value2 )
								{
									// in this case they have the same rank, thus it is pretty much the same
									return lhs < rhs;
								}
								else
								{
									// in this case the rank it is meaningful
									return value1 < value2;
								}
							}

							ranks_t* ref_to_rank_values;
					};


					/**
					 * @brief The typedef to the valid Operating Points containers
					 */
					using valid_ops_t = std::set < configuration_t, rank_comparator_t>;


					/****************************************************
					 * Constructors and destructor of the state
					 ****************************************************/

					/**
					 * @brief Default constructor
					 */
					state_internal_t( void );

					/**
					 * @brief Copy constructor
					 */
					state_internal_t( const state_internal_t& other) = delete;

					/**
					 * @brief Move constructor
					 */
					state_internal_t( state_internal_t&& other) = delete;

					/**
					 * @brief Assignment operator (copy semantic)
					 */
					state_internal_t& operator=(const state_internal_t& other) = delete;

					/**
					 * @brief Assignment operator (move semantic)
					 */
					state_internal_t& operator=(state_internal_t&& other) = delete;

					/**
					 * @brief Deconstructor
					 */
					~state_internal_t( void ) = default;



					/****************************************************
					 * Rank definition methods
					 ****************************************************/


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
						compute_rank.define_linear_rank(direction, op_fields... );
						referesh_valid_ops();
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
						compute_rank.define_geometric_rank(direction, op_fields... );
						referesh_valid_ops();
					}



					/**
					 * @brief Compute the reward of a configuration
					 *
					 * @param [in] configuration The evaluated configuration
					 *
					 * @result The reward [0,1], higher values are better
					 */
					float get_reward( const configuration_t& configuration);



					/****************************************************
					 * Operating Points manipulation methods
					 ****************************************************/


					/**
					 * @brief Get the knowledge from a knowledge base
					 *
					 * @param [in] kb The source knowledge base
					 *
					 * @details
					 * This method automatically issue a synch operation
					 * wrt the new knowledge_base.
					 */
					inline void set_knowledge_base( const knowledge_base_t& kb )
					{
						knowledge = kb.get_knowledge();
						synch();
					}

					/**
					 * @brief Add the Operating Points to the state
					 *
					 * @param [in] ops The list of Operating Points to be added
					 *
					 * @details
					 * This method is meant to be used when another object updates the global
					 * list of Operating Points.
					 * Usually it is called only if this state is the active one, since it
					 * requires to store the Operating Points that are introduced in the update.
					 *
					 * @note
					 * After calling this method the input parameter is not valid anymore
					 */
					void add_operating_points( operating_points_t& ops );


					/**
					 * @brief Removes the Operating Points from the state
					 *
					 * @param [in] ops The list of Operating Points to be removed
					 *
					 * @details
					 * This method is meant to be used when another object updates the global
					 * list of Operating Points.
					 * Usually it is called only if this state is the active one, since it
					 * requires to store the Operating Points that are removed in the update.
					 *
					 * @note
					 * After calling this method the input parameter is not valid anymore
					 */
					void remove_operating_points( operating_points_t& ops );


					/**
					 * @brief Synchronize the state with knowledge base
					 *
					 * @details
					 * The synchronization happens only if the knowledge base has been updated
					 * when another state was active.
					 */
					void synch( void );


					/**
					 * @brief Retrieve the best configuration
					 *
					 * @return A copy of the most suitable configuration
					 *
					 * @details
					 * The algorithm used is very simple:
					 *  - select the first configuration in the set (highest rank)
					 *  - if not available loop over the constraints from the bottom
					 *     - select the closer one between the configurations in the blocked_ops
					 */
					configuration_t get_best_configuration( void ) const;



					/**
					 * @brief Update the constraints according to the current situation
					 *
					 * @param [in] current_configuration The used configuration
					 *
					 * @param [in] without_monitor If true, the update does not consider the informations from the monitors
					 *
					 * @return True, if the internal structure has been updated
					 *
					 * @details
					 * This method updates the internal structures according to:
					 *   - the observed situation by the monitor (if any)
					 *   - changes in the goal value
					 */
					bool update( const configuration_t& current_configuration, const bool without_monitor = false );




					/****************************************************
					 * Constraints manipulation methods
					 ****************************************************/



					/**
					 * @brief Add a constraint on a parameter
					 *
					 * @param [in] field The name of the target field of the Operating Point
					 * @param [in] goal The reference to the target goal
					 * @param [in] priority The priority of the constraint
					 *
					 * @details
					 * The priority is used as an unique identifier of the constraint. Moreover it is used to
					 * sort the constraint, where to priority is expressed in a decreasing order.
					 * If two constraints are created with the same priority, the std::map semantics will apply.
					 */
					void add_parameter_constraint(const field_name_t field, goal_t& goal, const priority_t priority);




					/**
					 * @brief Add a constraint on a metric
					 *
					 * @param [in] field The name of the target field of the Operating Point
					 * @param [in] goal The reference to the target goal
					 * @param [in] priority The priority of the constraint
					 *
					 * @details
					 * The priority is used as an unique identifier of the constraint. Moreover it is used to
					 * sort the constraint, where to priority is expressed in a decreasing order.
					 * If two constraints are created with the same priority, the std::map semantics will apply.
					 */
					void add_metric_constraint(const field_name_t field, goal_t& goal, const priority_t priority);



					/**
					 * @brief Remove a constraint
					 *
					 * @param [in] priority The priority of the target constraint
					 */
					void remove_constraint(const priority_t priority);





					/****************************************************
					 * Utility methods
					 ****************************************************/

					/**
					 * @brief Clear the observation window
					 *
					 * @details
					 * This method clears the observation window of all the
					 * constraints on a metric observed a run-time
					 */
					void clear_monitors( void );

					/**
					 * @brief Test whether the effects of the configuration are observed
					 *
					 * @return True if all the monitors related to the constraints are full
					 */
					bool is_observable( void ) const;

					/**
					 * @brief Test whether the effects of the configuration are observed at all
					 *
					 * @return True if all the monitors related to the constraints are empty
					 */
					bool is_not_observable( void ) const;

					/**
					 * @brief Retrieve the vector of the observation errors
					 *
					 * @return a vector with observation errors
					 */
					observation_errors_t get_observation_errors( void ) const;

					/**
					 * @brief Set the vector of the observation errors
					 *
					 * @param [in] new_errors The vectors of new observation errors
					 */
					void set_observation_errors(observation_errors_t new_errors);




					/****************************************************
					 * Debug method
					 ****************************************************/

					/**
					 * @brief Dump the state of the constraint to the std out
					 */
					void dump( void ) const;


					/**
					 * @brief Check if the state is self coherent
					 *
					 * @details:
					 * If the state is not sel-coherent throws a runtime_error exception
					 */
					void sanity_checks( void ) const;


				private:

					/**
					 * @brief Reorder the op in the valid ops
					 *
					 * @details
					 * If the rank function changes, the already stored ops are sorted in
					 * the wrong order
					 */
					void referesh_valid_ops( void );



					/**
					 * @brief Update a single constraint data structure
					 *
					 * @param [in] constraint_it an iterator to the target constraint
					 * @param [in] op_range The range of Operating Points that are involved in the change
					 * @param [in] better If it is true it means that the situation might be better
					 *
					 * @return True, if the updates changed the constraint structure
					 *
					 * @details
					 * This methods updates the target constraint and the lower constraints according to the
					 * values observed by the monitors and changes in the goal value
					 */
					bool update_constraint( const constraints_t::iterator& constraint_it, const view_t::view_range_t& op_range, const bool better);



					/**
					 * @brief Update the state_t internal structures according to the new state
					 *
					 * @param [in] field The name of the target field of the Operating Point
					 * @param [in] goal The reference to the target goal
					 * @param [in] priority The priority of the constraint
					 *
					 * @details
					 * The priority is used as an unique identifier of the constraint. Moreover it is used to
					 * sort the constraint, where to priority is expressed in a decreasing order.
					 * If two constraints are created with the same priority, the std::map semantics will apply.
					 */
					void add_constraint_update(const constraints_t::iterator new_constraint_it, goal_t::target_t_ptr& goal);



					/**
					 * @brief Emplace in the rank containers an Operating Point
					 *
					 * @param [in] op The target Operating Point
					 *
					 * @details
					 * It also update the reference to the worst and best Operating Point according to the rank
					 */
					void insert_rank_value( const operating_point_t& op );


					/**
					 * @brief Finds the best and the worst Operating Point according to the rank value
					 */
					void find_worst_and_best_ops( void );






					/****************************************************
					 * Required attribute for the state
					 ****************************************************/

					/**
					 * @brief The comparator object used to compare two configurations
					 */
					rank_comparator_t rank_functor;

					/**
					 * @brief The list of constraints used to filter out invalid ops
					 */
					constraints_t constraints;

					/**
					 * @brief Relates a configuration to a rank value
					 */
					ranks_t ranks;

					/**
					 * @brief Contains all the valid configurations, ordered by rank
					 */
					valid_ops_t valid_ops;

					/**
					 * @brief The object used to compute the rank
					 */
					rank_calculator_t compute_rank;


					/**
					 * @brief A reference to the knowledge base
					 */
					knowledge_base_t::knowledge_t_ptr knowledge;

					/**
					 * @brief The version of the knowledge in this state
					 *
					 * @details
					 * Used in case the knowledge base is updated while the manager was in
					 * another state.
					 */
					knowledge_base_t::version_t version;


#ifdef MARGOT_ENABLE_REWARD_COMPUTATION

					/****************************************************
					 * Attributes for computing the reward of the state
					 ****************************************************/

					/**
					 * @brief A reference to the worst configuration according to the rank
					 */
					ranks_t::iterator worst_operating_point;

					/**
					 * @brief A reference to the best configuration according to the rank
					 */
					ranks_t::iterator best_operating_point;

					/**
					 * @brief A flag to signal if the reference to the worst and best OPs are valid
					 */
					bool valid_rank_iterator;

#endif // MARGOT_ENABLE_REWARD_COMPUTATION
			};

		private:

			/**
			* @brief A typedef to the pointer of a state
			*/
			using state_internal_ptr_t = std::shared_ptr<state_internal_t>;

			/**
			 * @brief a reference to the internal state structure
			 */
			state_internal_ptr_t state;
	};

}

#endif // MARGOT_ASRTM_STATE_HEADER
