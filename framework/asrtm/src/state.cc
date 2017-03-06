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
#include <stdexcept>
#include <iostream>
#include <cstdio>
#include <cstddef>
#include <cmath>


#include <margot/state.hpp>






namespace margot
{

	constexpr float state_t::reward_balance_coefficiet;
	constexpr float state_t::reward_max_value;


	state_t::observation_errors_t state_t::state_internal_t::get_observation_errors( void ) const
	{
		observation_errors_t result;

		for ( auto && c : constraints)
		{
			// add the observation error, if they have a monitor associated
			if (c.second.goal->monitor_ptr)
			{
				result.emplace(c.second.metric_view->get_field_name(), c.second.previous_observation_error);
			}
		}

		return result;
	}

	void state_t::state_internal_t::set_observation_errors(observation_errors_t new_errors)
	{
		for ( auto& c : constraints)
		{
			// get a reference to the interested element
			const auto&& pointer = new_errors.find(c.second.metric_view->get_field_name());

			// set the observation error if needed
			if (c.second.goal->monitor_ptr && (pointer != new_errors.end()))
			{
				c.second.previous_observation_error = pointer->second;
			}
		}
	}





#ifdef MARGOT_ENABLE_REWARD_COMPUTATION

	state_t::state_internal_t::state_internal_t( void ): rank_functor(
	{
		&ranks
	}), valid_ops(rank_functor), version(std::chrono::steady_clock::now()), valid_rank_iterator(false) {}



	float state_t::state_internal_t::get_reward( const configuration_t& configuration)
	{
		// compute the constraint factor
		float constraint_factor_num = 0.0f;
		float constraint_factor_den = 0.0f;
		const int num_constraints = constraints.size() - 1;
		int counter = 0;


		// handle the case if there are no constraints
		if (constraints.empty())
		{
			constraint_factor_num = 1.0f;
			constraint_factor_den = 1.0f;
		}
		else
		{
			// otherwise loop over all the availble constraints
			for (const auto& constraint : constraints)
			{
				// get the required values
				const margot_value_t goal_value = constraint.second.previous_goal_value;
				const operating_point_t op = knowledge->get_operating_point(configuration);
				const margot_value_t configuration_value = constraint.second.metric_view->extract_op_value(op);

				// check if the constraint is actually met
				const bool valid = constraint.second.goal->compare(configuration_value, goal_value);

				// update the numerator of the constraints factor
				if (!valid)
				{
					const margot_value_t min_value = std::min(constraint.second.metric_view->get_minimum_value(), goal_value);
					const margot_value_t max_value = std::max(constraint.second.metric_view->get_maximum_value(), goal_value);
					const float total_distance = std::abs(max_value - min_value);
					const float regret_distance = std::abs(goal_value - configuration_value);
					const float penality = total_distance == 0 ? 0.0f : regret_distance / total_distance;
					constraint_factor_num += std::pow(2.0f, static_cast<float>(num_constraints - counter)) * (1.0f - penality);
				}
				else
				{
					constraint_factor_num += std::pow(2.0f, static_cast<float>(num_constraints - counter));
				}

				// update the denominator of the constraints factor
				constraint_factor_den += std::pow(2.0f, static_cast<float>(num_constraints - counter));

				// update the counter
				counter += 1;
			}
		}

		const float constraint_factor = (constraint_factor_num) / (constraint_factor_den) * margot::state_t::reward_balance_coefficiet;


		// compute the rank factor (if needed)
		if (!valid_ops.empty())
		{
			// get the extreme value of the rank
			if (!valid_rank_iterator)
			{
				find_worst_and_best_ops();
			}

			// compute the rank factor
			const float total_distance = static_cast<float>(std::abs(worst_operating_point->second - best_operating_point->second));
			const float regret_distance = static_cast<float>(std::abs(ranks.at(configuration) - worst_operating_point->second));
			const float rank_factor = total_distance == 0 ? 1.0f : regret_distance / total_distance;
			const float balanced_reward = constraint_factor + (rank_factor * (1.0f - margot::state_t::reward_balance_coefficiet));

			// return the complete reward
			return balanced_reward * margot::state_t::reward_max_value;
		}

		// return the reward without the rank factor
		return constraint_factor * margot::state_t::reward_max_value;
	}



	void state_t::state_internal_t::insert_rank_value(const operating_point_t& op)
	{
		// compute the rank value of the Operating Point
		const rank_t rank_value = compute_rank(op);

		// check for the minimum and the maximum
		bool is_the_new_worst = false;
		bool is_the_new_best = false;

		if (valid_rank_iterator)
		{
			if (rank_value > worst_operating_point->second)
			{
				is_the_new_worst = true;
			}

			if (rank_value < best_operating_point->second)
			{
				is_the_new_best = true;
			}
		}

		// check for the empty container
		const bool ranks_empty = ranks.empty();

		// insert the new rank
		const auto result_insertion = ranks.emplace(op.first, rank_value);

		// if needed update the references
		if (is_the_new_best || ranks_empty)
		{
			best_operating_point = result_insertion.first;
			valid_rank_iterator = true;
		}

		if (is_the_new_worst || ranks_empty)
		{
			worst_operating_point = result_insertion.first;
			valid_rank_iterator = true;
		}
	}

	void state_t::state_internal_t::find_worst_and_best_ops( void )
	{
		valid_rank_iterator = !ranks.empty();

		if (valid_rank_iterator)
		{
			// assume that the first elements are the best/worst
			auto conf_it = ranks.begin();
			worst_operating_point = conf_it;
			best_operating_point = conf_it;

			// loop over the remainder of the list
			for ( ++conf_it; conf_it != ranks.end(); ++conf_it)
			{
				if (conf_it->second < best_operating_point->second)
				{
					best_operating_point = conf_it;
				}

				if (conf_it->second > worst_operating_point->second)
				{
					worst_operating_point = conf_it;
				}
			}

			// set the we have found the new best & worst ops
			valid_rank_iterator = true;
		}
	}

#else // MARGOT_ENABLE_REWARD_COMPUTATION

	state_t::state_internal_t::state_internal_t( void ): rank_functor(
	{
		&ranks
	}), valid_ops(rank_functor), version(std::chrono::steady_clock::now()) {}


	float state_t::state_internal_t::get_reward( const configuration_t& configuration)
	{
		return state_t::reward_max_value;
	}

	void state_t::state_internal_t::insert_rank_value(const operating_point_t& op)
	{
		ranks.emplace(op.first, compute_rank(op));
	}

	void state_t::state_internal_t::find_worst_and_best_ops( void )
	{
	}


#endif // MARGOT_ENABLE_REWARD_COMPUTATION



	void state_t::state_internal_t::add_operating_points( operating_points_t& ops )
	{
		// loop over the Operating Points
		for ( auto && op : ops )
		{
			// Insert the op in the rank container
			insert_rank_value(op);

			// used to check if the OP is valid
			bool valid = true; /* in case there are no constraints */

			// loop over the constraint
			for ( auto& constraint : constraints )
			{
				// get the value of the field
				const margot_value_t op_value = constraint.second.metric_view->extract_op_value(op);

				// check if the op satisfy the goal
				// NOTE: since we just add the OPs and we don't update the constraint
				// the correct value to use in order to test whether the goal is
				// achieved or not, must refer to previous known situation.
				// In this case we a consistent representation of the problem.
				valid = constraint.second.goal->compare(op_value, constraint.second.previous_goal_value);

				// if it is not valid
				if (!valid)
				{
					// add it to the blocking set
					constraint.second.blocked_ops.emplace(std::move(op.first));

					// do not iterate anymore
					break;
				}
			}

			// if it is valid for all the constraint, add it to set of valid points
			if (valid)
			{
				valid_ops.emplace(std::move(op.first));
			}
		}

		// synchronize with the knowledge base
		version = knowledge->get_version();
	}





	void state_t::state_internal_t::remove_operating_points(operating_points_t& ops)
	{
		// loop over the OPs to remove
		for ( auto && op : ops )
		{
			bool removed = false;

#ifdef MARGOT_ENABLE_REWARD_COMPUTATION

			// check if we are removing the best or the worst Operating Point
			if (valid_rank_iterator)
			{
				if ((op.first == worst_operating_point->first) || (op.first == best_operating_point->first))
				{
					valid_rank_iterator = false;
				}
			}

#endif // MARGOT_ENABLE_REWARD_COMPUTATION

			// loop over the constraints
			for ( auto& constraint : constraints )
			{
				// try to remove the OP
				const std::size_t num_erased_op = constraint.second.blocked_ops.erase(op.first);

				if (num_erased_op > 0)
				{
					removed = true;
					break;
				}
			}

			// remove it from the valid ones
			if (!removed)
			{
				valid_ops.erase(op.first);
			}

			// remove the Operating Points also from the ranks structure
			ranks.erase(std::move(op.first));
		}

		// synchronize with the knowledge base
		version = knowledge->get_version();
	}







	void state_t::state_internal_t::synch( void )
	{
		// check if the state is updated
		if (version == knowledge->get_version())
		{
			// nothing to do
			return;
		}

		// otherwise clear the global data structures
		ranks.clear();
		valid_ops.clear();
#ifdef MARGOT_ENABLE_REWARD_COMPUTATION
		valid_rank_iterator = false;
#endif // MARGOT_ENABLE_REWARD_COMPUTATION

		// clear the blocked_list in all the constraints
		for ( auto& constraint : constraints )
		{
			constraint.second.blocked_ops.clear();
		}

		// get the new version
		version = knowledge->get_version();

		// get the list of the new OPs
		const auto knowledge_range = knowledge->get_knowlege_range();

		// loop over the known configurations
		for ( auto op_it = knowledge_range.first; op_it != knowledge_range.second; ++op_it )
		{

			// compute the rank
			insert_rank_value(*op_it);

			// used to check if the OP is valid
			bool valid = true; /* in case there are no constraints */

			// loop over the constraint
			for ( auto& constraint : constraints )
			{
				// get the value of the field
				const margot_value_t op_value = constraint.second.metric_view->extract_op_value(*op_it);

				// check if the op satisfy the goal
				// NOTE: we are rebuilding the whole data structure, however we have no clue on the
				// OP actually used, thus we must rely on the previous value
				valid = constraint.second.goal->compare(op_value, constraint.second.previous_goal_value);

				// if it is not valid
				if (!valid)
				{
					// add it to the blocking set
					constraint.second.blocked_ops.emplace(op_it->first);

					// do not iterate anymore
					break;
				}
			}

			// if it is valid for all the constraint, add it to set of valid points
			if (valid)
			{
				valid_ops.emplace(op_it->first);
			}
		}

	}






	void state_t::state_internal_t::referesh_valid_ops( void )
	{
		// compute the rank of all the Operating Points
		ranks.clear();

#ifdef MARGOT_ENABLE_REWARD_COMPUTATION
		valid_rank_iterator = false;
#endif // MARGOT_ENABLE_REWARD_COMPUTATION
		auto op_range = knowledge->get_knowlege_range();

		for ( auto op_it = op_range.first; op_it != op_range.second; ++op_it )
		{
			insert_rank_value(*op_it);
		}

		// check if we need to refresh the list
		if (valid_ops.empty())
		{
			return;
		}

		// store temporarly the valid OP
		valid_ops_t temp_list(rank_functor);
		temp_list.swap(valid_ops);

		// repopulate it
		for ( auto && configuration : temp_list )
		{
			valid_ops.emplace(std::move(configuration));
		}
	}












	configuration_t state_t::state_internal_t::get_best_configuration( void ) const
	{
		//========= Check if there are valid Operating Points

		if (!valid_ops.empty())
		{
			// the first of that set is the best one
			return *valid_ops.begin();
		}




		//========= Otherwise find the unsatisfiable constraint

		// otherwise get the reference to the last constraint
		auto constraint_it = constraints.cend();
		std::advance(constraint_it, -1);

		// loop backward until there is a non-empty constraint
		while ( constraint_it->second.blocked_ops.empty() )
		{
			if (pedantic_check())
			{
				if (constraint_it == constraints.cbegin() && constraint_it->second.blocked_ops.empty())
				{
					throw std::logic_error("[state_t] Error: Unexpected situation: No valid ops and the constraints do not block any op");
				}
			}

			std::advance(constraint_it, -1);
		}



		//========= The best OP is between the blocked ones

		operating_points_t proposed_ops;

		// assume that the first configuration is the best
		const auto first_op = knowledge->get_operating_point(*(constraint_it->second.blocked_ops.cbegin()));
		margot_value_t closer_value = constraint_it->second.metric_view->extract_op_value(first_op);
		proposed_ops.emplace_back(std::move(first_op));

		// take a reference to the compare function of the goal of the evaluated constraint
		const auto& better = constraint_it->second.goal->compare;

		// loop over the remainder of the configurations
		for ( const auto& configuration : constraint_it->second.blocked_ops )
		{
			// evaluate the configuration
			const auto evaluated_op = knowledge->get_operating_point(configuration);
			const margot_value_t value_evaluated_op = constraint_it->second.metric_view->extract_op_value(evaluated_op);

			// check if it is the closer
			if (!better(closer_value, value_evaluated_op))
			{
				// discard all the other configurations
				if (closer_value != value_evaluated_op)
				{
					proposed_ops.clear();
				}

				// inset the new value
				proposed_ops.emplace_back(std::move(evaluated_op));

				// update the closer value
				closer_value = value_evaluated_op;
			}
		}

		// boil out if there is only one Operating Point
		if (proposed_ops.size() == 1)
		{
			return std::move(proposed_ops.front().first);
		}




		//========= Otherwise narrow down the proposed OPs using the lower constraints

		// go one constraint down
		std::advance(constraint_it, -1);

		while (proposed_ops.size() > 1 && constraint_it != constraints.cend())
		{
			// swap out the proposed op list to a temporary one
			operating_points_t temp_list;
			temp_list.swap(proposed_ops);

			// get the comparator function
			const auto& compare = constraint_it->second.goal->compare;

			// assuming the best the first op
			proposed_ops.emplace_back(std::move(temp_list.front()));
			temp_list.pop_front();
			margot_value_t previous_value = constraint_it->second.metric_view->extract_op_value(*proposed_ops.cbegin());
			const margot_value_t goal_value = constraint_it->second.previous_goal_value;
			bool satisfied = compare(previous_value, goal_value);


			// loop on the ramaining OPs
			for ( auto && op : temp_list )
			{
				// evaluate the OP
				const margot_value_t current_op_value = constraint_it->second.metric_view->extract_op_value(op);

				// check if the new OP satisfy the goal
				if (compare(current_op_value, goal_value))
				{
					// check if we need to clear the list
					if (!satisfied)
					{
						proposed_ops.clear();
					}

					// add the OP
					proposed_ops.emplace_back(std::move(op));
					satisfied = true;
				}
				else
				{
					// check if the constraint is already satisfied
					if (!satisfied)
					{
						// check if the current OP is actually better
						if (!compare(previous_value, current_op_value))
						{
							// we found the new best
							if (previous_value != current_op_value)
							{
								proposed_ops.clear();
							}

							proposed_ops.emplace_back(std::move(op));
						}
					}
				}
			}

			// go to the lower constraint
			++constraint_it;
		}

		// boil out if there is only one configuration
		if (proposed_ops.size() == 1)
		{
			return std::move(proposed_ops.front().first);
		}



		//========= Otherwise narrow down the proposed OPs using the rank
		auto best_configuration = std::move(proposed_ops.front().first);
		rank_t best_rank = ranks.at(best_configuration);
		proposed_ops.pop_front();

		for (auto && op : proposed_ops)
		{
			// evaluate the op
			const rank_t evaluated_op_rank = ranks.at(op.first);

			if (evaluated_op_rank < best_rank)
			{
				best_configuration = std::move(op.first);
				best_rank = evaluated_op_rank;
			}
		}

		return best_configuration;
	}













	bool state_t::state_internal_t::update( const configuration_t& current_configuration, const bool without_monitor )
	{
		// assume that there are no changes
		bool changed = false;

		// get the Operating Point used in the run
		const auto current_op = without_monitor ? operating_point_t{} :
		                        knowledge->get_operating_point(current_configuration);

		// loop over the constraints starting from the higher priority one
		for ( auto constraint_it = constraints.begin(); constraint_it != constraints.end(); ++constraint_it )
		{
			// retrieve the new goal information
			margot_value_t new_goal_value = static_cast<margot_value_t>( constraint_it->second.goal->value );

			// initialze the observation error as the previous one
			margot_value_t observation_error = constraint_it->second.previous_observation_error;

			// retrieve the observed metric value (without considering that we have removed the current OP)
			if (!without_monitor)
			{
				margot_value_t observed_value;
				const bool has_new_information = constraint_it->second.goal->observed_value(observed_value);

				// update the constraint if needed
				if (has_new_information)
				{
					// get the expected value
					const margot_value_t expected_value = constraint_it->second.metric_view->extract_op_value(current_op);

					// avoid the zero trap ( we assume that if observed value is zero, the expected one is also zero )
					if (observed_value != 0)
					{
						// update the observation error
						observation_error = expected_value / observed_value;
						constraint_it->second.previous_observation_error = observation_error;
					}
				}
			}

			// scale the "nominal" goal value
			new_goal_value *= observation_error;

			// get the range of the involved OPs
			const auto ops_changed_range = constraint_it->second.metric_view->range(constraint_it->second.previous_goal_value, new_goal_value);

			// check if the situation is going better
			const bool better = constraint_it->second.goal->compare(constraint_it->second.previous_goal_value, new_goal_value);

			// update the constraint goal value
			constraint_it->second.previous_goal_value = new_goal_value;

			// update the constraint
			const bool constraint_changed = update_constraint(constraint_it, ops_changed_range, better);

			// update the changed flag
			changed = changed || constraint_changed;
		}

		return changed;
	}









	bool state_t::state_internal_t::update_constraint(const constraints_t::iterator& constraint_it, const view_t::view_range_t& op_range, const bool better)
	{
		// assume no changes
		bool changed = false;

		// handle the case where the situation is going better
		if (better)
		{
			// loop over the OPs that are involved in the change
			for ( auto view_map_it = op_range.first; view_map_it != op_range.second; ++view_map_it)
			{
				// check if the OP is valid for the constraint
				if (constraint_it->second.goal->compare(view_map_it->first, constraint_it->second.previous_goal_value))
				{
					// check if the OP is blocked by this constraint
					lookup_table_t::iterator element = constraint_it->second.blocked_ops.find(view_map_it->second);

					// if it is; remove it from this list and propagate it at the lower constraint
					if (element != constraint_it->second.blocked_ops.end())
					{
						// update changed
						changed = true;

						// retrieve the OP
						auto op = knowledge->get_operating_point(view_map_it->second);

						// erase the configuration
						constraint_it->second.blocked_ops.erase(element);

						// loop over the constraint assuming that the OP is valids
						bool valid = true;

						for ( auto c_it = std::next(constraint_it, 1); c_it != constraints.end(); ++c_it)
						{
							// get the value of the OP
							const margot_value_t value = c_it->second.metric_view->extract_op_value(op);

							// check if it is not valid
							if (!c_it->second.goal->compare(value, c_it->second.previous_goal_value))
							{
								// add it to the blocked ops
								c_it->second.blocked_ops.emplace(std::move(op.first));

								// update the validity
								valid = false;

								// boil out
								break;
							}
						}

						// add it to the valid set, if it is valid
						if (valid)
						{
							valid_ops.emplace(std::move(op.first));
						}
					}
				}
			}
		}
		else // the situation is getting worst
		{
			// loop over the OPs that are involved in the change
			for ( auto view_map_it = op_range.first; view_map_it != op_range.second; ++view_map_it)
			{
				// check if the OP is valid for the constraint
				if (!constraint_it->second.goal->compare(view_map_it->first, constraint_it->second.previous_goal_value))
				{
					// check if the OP is blocked by this constraint
					lookup_table_t::iterator element = constraint_it->second.blocked_ops.find(view_map_it->second);

					// if it is not; add it to blocked and remove it from one of the other set
					if (element == constraint_it->second.blocked_ops.end())
					{
						// update the changed flag
						changed = true;

						// variable to check if we have removed the OP
						bool removed = false;

						// loop over the lower constraint
						for ( auto c_it = std::next(constraint_it, 1); c_it != constraints.end(); ++c_it)
						{
							// check if the configuration is present
							auto element = c_it->second.blocked_ops.find(view_map_it->second);

							// if so removed it
							if (element != c_it->second.blocked_ops.end())
							{
								removed = true;
								c_it->second.blocked_ops.erase(element);
								break;
							}
						}

						// otherwise try to remove it by the set of valid Operating Points
						if (!removed)
						{
							// try to remove it from the list of the invalid ones
							const std::size_t num_removed_items = valid_ops.erase(view_map_it->second);

							// if we have done it, we might add it in the list of the blocked ones
							if (num_removed_items > 0)
							{
								constraint_it->second.blocked_ops.emplace(std::move(view_map_it->second));
							}
						}
						else
						{
							// insert the configuration in the blocked list
							constraint_it->second.blocked_ops.emplace(std::move(view_map_it->second));
						}
					}
				}
			}
		}

		return changed;
	}









	void state_t::state_internal_t::add_parameter_constraint(const field_name_t field, goal_t& goal, const priority_t priority)
	{
		// check if there is a knowledge base
		if (pedantic_check())
			if (!knowledge)
			{
				throw std::runtime_error("[state_t] Error: Unable to add a Constraint without a knowledge base");
			}

		// initialize the constraint
		constraint_t new_constraint;
		new_constraint.goal = goal.get_target();                                 // the goal structure
		new_constraint.metric_view = knowledge->get_parameter_view(field);       // the view on the parameter
		new_constraint.previous_goal_value = new_constraint.goal->value;         // the actual goal value
		new_constraint.previous_observation_error = static_cast<margot_value_t>(1);// assuming our estimate value is correct

		// insert it in the list
		constraints.emplace(priority, std::move(new_constraint));

		// get a reference to the created constraint
		constraints_t::iterator new_constraint_it = constraints.find(priority);

		// update the internal data structure
		add_constraint_update(new_constraint_it, new_constraint_it->second.goal);
	}


	void state_t::state_internal_t::add_metric_constraint(const field_name_t field, goal_t& goal, const priority_t priority)
	{
		// check if there is a knowledge base
		if (pedantic_check())
			if (!knowledge)
			{
				throw std::runtime_error("[state_t] Error: Unable to add a Constraint without a knowledge base");
			}

		// initialize the constraint
		constraint_t new_constraint;
		new_constraint.goal = goal.get_target();                                 // the goal structure
		new_constraint.metric_view = knowledge->get_metric_view(field);          // the view on the metric
		new_constraint.previous_goal_value = new_constraint.goal->value;         // the actual goal value
		new_constraint.previous_observation_error = static_cast<margot_value_t>(1);// assuming our estimate value is correct

		// insert it in the list
		constraints.emplace(priority, std::move(new_constraint));

		// get a reference to the created constraint
		constraints_t::iterator new_constraint_it = constraints.find(priority);

		// update the internal data structure
		add_constraint_update(std::move(new_constraint_it), new_constraint_it->second.goal);
	}




	void state_t::state_internal_t::add_constraint_update(const constraints_t::iterator new_constraint_it, goal_t::target_t_ptr& goal)
	{
		// get the blocked ops from the lower constraints
		for ( auto it_c = std::next(new_constraint_it); it_c != constraints.end(); ++it_c )
		{
			// loop over the blocked OP of the constraint
			for ( auto op_it = it_c->second.blocked_ops.begin(); op_it != it_c->second.blocked_ops.end(); /** internally handled **/ )
			{
				// obtain the OP equivalent
				const auto op = knowledge->get_operating_point(*op_it);

				// obtain the value
				const margot_value_t value = new_constraint_it->second.metric_view->extract_op_value(op);

				// check if the goal is not achieved
				if (!goal->compare(value, goal->value))
				{
					// move the configuration in the new constraint
					new_constraint_it->second.blocked_ops.emplace(std::move(*op_it));

					// remove from the lower constraint
					op_it = it_c->second.blocked_ops.erase(op_it);
				}
				else
				{
					// go to the next op
					++op_it;
				}
			}
		}

		// loop over the valid ops
		for ( auto op_it = valid_ops.begin(); op_it != valid_ops.end(); /** internally handled **/ )
		{
			// obtain the Operating Point
			const auto op = knowledge->get_operating_point(*op_it);

			// get the value
			const margot_value_t value = new_constraint_it->second.metric_view->extract_op_value(op);

			// check if it is not satisfied
			if (!goal->compare(value, goal->value))
			{
				// add it to the blocked ops
				new_constraint_it->second.blocked_ops.emplace(std::move(*op_it));

				// remove it from the valid ops
				op_it = valid_ops.erase(op_it);
			}
			else
			{
				// go to the next op
				++op_it;
			}
		}
	}




	void state_t::state_internal_t::remove_constraint(const priority_t priority)
	{
		// get the reference to the constraint
		constraints_t::iterator removed_constraint = constraints.find(priority);

		// bail out if it is not found
		if (removed_constraint == constraints.end())
		{
			return;
		}

		// loop over its blocked configuration
		for ( auto && configuration : removed_constraint->second.blocked_ops )
		{
			// assume it is valid
			bool valid = true;

			// get the OP
			const auto op = knowledge->get_operating_point(configuration);

			// loop through the lower constraints
			for ( auto constraint_it = std::next(removed_constraint); constraint_it != constraints.end(); ++constraint_it )
			{
				// get the value
				const margot_value_t value = constraint_it->second.metric_view->extract_op_value(op);

				// check if it is not valid
				if (!constraint_it->second.goal->compare(value, constraint_it->second.previous_goal_value))
				{
					// add the configuration on the blocked ops and boil out
					constraint_it->second.blocked_ops.emplace(std::move(configuration));
					valid = false;
					break;
				}
			}

			// add it to the valid set if it is valid
			if (valid)
			{
				valid_ops.emplace(std::move(configuration));
			}
		}

		// actually remove the constraint
		constraints.erase(removed_constraint);
	}


	void state_t::state_internal_t::clear_monitors( void )
	{
		for ( auto c : constraints )
		{
			c.second.goal->clear();
		}
	}

	bool state_t::state_internal_t::is_observable( void ) const
	{
		// check the result
		for ( const auto& constraint : constraints )
		{
			// reference to the monitor
			const auto monitor_ref = constraint.second.goal->monitor_ptr;

			// check if the goal has a monitor
			if (monitor_ref)
			{
				// check if it is fully observable
				if (!monitor_ref->full())
				{
					return false;
				}
			}
		}

		// otherwise the state is fully observed
		return true;
	}


	bool state_t::state_internal_t::is_not_observable( void ) const
	{
		// loop over the constraints
		for ( const auto& constraint : constraints )
		{
			// reference to the monitor
			const auto monitor_ref = constraint.second.goal->monitor_ptr;

			// check if the goal has a monitor
			if (monitor_ref)
			{
				// check if it is fully observable
				if (!monitor_ref->empty())
				{
					return false;
				}
			}
		}

		// otherwise the state is fully observed
		return true;
	}



	void state_t::state_internal_t::sanity_checks( void ) const
	{
		if (knowledge)
		{
			// check the version
			if (knowledge->get_version() != version )
			{
				throw std::runtime_error("Error: the state is out of synch wrt the knowledge base");
			}

			// get the number of ops
			const std::size_t num_op = knowledge->size();

			// check the number of ranked ops
			if ( ranks.size() != num_op )
			{
				throw std::runtime_error("Error: the number of ranked OPs differs wrt the ones in the knowledge base");
			}

			// check the number of blocked + valid ops
			std::size_t evaluated_ops = valid_ops.size();

			for (const auto& constraint : constraints )
			{
				evaluated_ops += constraint.second.blocked_ops.size();
			}

			if (num_op != evaluated_ops)
			{
				throw std::runtime_error("Error: the number of managed OPs differes wrt the ones in the knowledge base");
			}


			if (ranks.size() != evaluated_ops )
			{
				throw std::runtime_error("Error: the number of managed OPs differs wrt the ranked OPs");
			}
		}
	}




	void state_t::state_internal_t::dump( void ) const
	{
		using std::cout;
		using std::endl;

		// Helper function
		auto dump_configuration = [] (const configuration_t& conf)
		{
			printf("\t+----------------------+\n");

			for ( const auto c : conf)
			{
				printf("\t|% 22.2f|\n", c);
			}

			printf("\t+----------------------+\n");
		};

		cout << endl;

		// print the header
		cout << "********************************************************************" << endl;
		cout << "***                          STATE DUMP                          ***" << endl;
		cout << "********************************************************************" << endl;

		cout << endl;

		// print the overall status
		cout << "=================================================[GENERAL STATUS]===" << endl;
		cout << "version:          " << version.time_since_epoch().count() << endl;
		cout << "num constraints:  " << constraints.size() << endl;
		cout << "num ranked OPs:   " << ranks.size() << endl;
		cout << "num valid OPs:    " << valid_ops.size() << endl;
		cout << "knowledge ptr:    " << knowledge << endl;
#ifdef MARGOT_ENABLE_REWARD_COMPUTATION
		cout << "best rank value:  ";

		if (valid_rank_iterator)
		{
			cout << best_operating_point->second << endl;
		}
		else
		{
			cout << "N/A" << endl;
		}

		cout << "worst rank value:  ";

		if (valid_rank_iterator)
		{
			cout << worst_operating_point->second << endl;
		}
		else
		{
			cout << "N/A" << endl;
		}

#endif // MARGOT_ENABLE_REWARD_COMPUTATION

		cout << endl;

		// perform sanity checks
		cout << "==================================================[SANITY CHECKS]===" << endl;

		if (knowledge)
		{
			// check the version
			if (knowledge->get_version() == version )
			{
				cout << "version synch [OK]" << endl;
			}
			else
			{
				cout << "version synch [FAIL] -> knowledge version: " << knowledge->get_version().time_since_epoch().count() << endl;
			}

			// get the number of ops
			const std::size_t num_op = knowledge->size();

			// check the number of ranked ops
			if ( ranks.size() == num_op )
			{
				cout << "num ranked OPs [OK]" << endl;
			}
			else
			{
				cout << "num ranked OPs [FAIL] -> total OPs: " << num_op << " | ranked OPs: " << ranks.size() << endl;
			}

			// check the number of blocked + valid ops
			std::size_t evaluated_ops = valid_ops.size();

			for (const auto& constraint : constraints )
			{
				evaluated_ops += constraint.second.blocked_ops.size();
			}

			if (num_op == evaluated_ops)
			{
				cout << "num considered OPs [OK]" << endl;
			}
			else
			{
				cout << "num considered OPs [FAIL] -> total OPs: " << num_op << " | considered OPs: " << evaluated_ops << endl;
			}


			// check self-consistency
			if (ranks.size() == evaluated_ops )
			{
				cout << "self consistency [OK]" << endl;
			}
			else
			{
				cout << "self consistency [FAIL] -> considered OPs: " << evaluated_ops << " | ranked OPs " << ranks.size() << endl;
			}
		}
		else
		{
			cout << "[ERROR]: The knowledge base is a NULL pointer" << endl;
		}



		// dump the state internal structure
		cout << endl;
		cout << "==================================================[INTERNAL DUMP]===" << endl;

		cout << endl;

		if (!constraints.empty())
		{
			cout << " The following is the list of constraints defined in the state." << endl;
			cout << " For each constraints it prints the blocked configurations" << endl;
			cout << " After each configuration, it ptints its value." << endl;
		}
		else
		{
			cout << "<NO CONSTRAINTS DEFINED>" << endl;
		}

		for ( const auto& constraint_it : constraints)
		{
			cout << endl;
			// Print the constraint status
			cout << "===[Constraint with priority " << constraint_it.first <<  "]===" << endl;
			cout << "Actual target value: " << constraint_it.second.goal->value << endl;
			cout << "Previous goal value: " << constraint_it.second.previous_goal_value << endl;
			cout << "View ptr: " << constraint_it.second.metric_view << endl;
			cout << "Num blocked OPs: " << constraint_it.second.blocked_ops.size() << endl;

			for ( const auto& conf : constraint_it.second.blocked_ops )
			{
				// get the OP
				const auto op = knowledge->get_operating_point(conf);

				// get the value
				const margot_value_t value = constraint_it.second.metric_view->extract_op_value(op);

				// dump the OP
				dump_configuration(conf);

				// print the value
				printf("\t|% 22.2f|\n", value);
				printf("\t+----------------------+\n");
				cout << endl;
			}
		}

		cout << endl;
		cout << "######### VALID CONFIGURATIONS #########" << endl << endl;

		if (!valid_ops.empty())
		{
			cout << " The following is the list of OPs that are valid." << endl;
			cout << " After each configuration, it prints its rank." << endl;
			cout << endl;
		}
		else
		{
			cout << "<NO VALID OPERATING POINTS>" << endl;
		}

		for ( const auto& conf : valid_ops )
		{
			// get the OP
			const auto op = knowledge->get_operating_point(conf);

			// get the value
			const margot_value_t value = ranks.at(conf);

			// dump the OP
			dump_configuration(conf);

			// print the value
			printf("\t|% 22.2f|\n", value);
			printf("\t+----------------------+\n");
			cout << endl;
		}

		cout << endl;
		cout << "*****************************END************************************" << endl;
	}

}
