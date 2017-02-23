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

#include <algorithm>
#include <iterator>
#include <limits>
#include <cmath>

#include "margot/mab_sw_ucb.hpp"
#include "margot/state.hpp"


#ifdef MARGOT_LEARNING_ENABLE_FILE_LOG
	#define MARGOT_LOG(x) sw_mab_t::outfile << x << std::endl
#else
	#define MARGOT_LOG(x)
#endif // LEARNING_ENABLE_FILE_LOG


namespace margot
{

	static constexpr float initializing_reward = state_t::reward_max_value * 10;

#ifdef MARGOT_LEARNING_ENABLE_FILE_LOG
	std::ofstream sw_mab_t::outfile;
#endif // LEARNING_ENABLE_FILE_LOG

	void sw_mab_t::explode_configurations(const learning_configurations_t& configurations,
	                                      configuration_t evaluated_configuration,
	                                      const float rank_value = 0.0,
	                                      const std::size_t depth )
	{
		// check if we have completed the configuration
		if (depth == configurations.size())
		{
			// create the arm
			choices.emplace_back(arm_ptr_t(new arm_t(evaluated_configuration, uncertainty_coefficient, rank_value)));
			MARGOT_LOG("Created an arm with a configuration size of " << evaluated_configuration.size() << " and rank " << rank_value << ":" );

			// update the min and max rank values
			min_rank_value = std::min(min_rank_value, rank_value);
			max_rank_value = std::max(max_rank_value, rank_value);

			// update the lookup table
			arm_lookup_table.emplace(evaluated_configuration, choices.size() - 1);

			// configuration stored
			return;
		}

		// otherwise get the reference to the values of the lower levels
		const knob_values_t& target_values = configurations[depth].second;


		// loop over them
		for ( auto value_it = target_values.cbegin(); value_it != target_values.cend(); ++value_it )
		{
			// get the next level configuration
			configuration_t my_next_level_configuration;

			// copy the previous values
			std::copy(evaluated_configuration.begin(), evaluated_configuration.end(), std::back_inserter(my_next_level_configuration));

			// compute the rank value of this particular value
			const float rank_value_element = (*value_it) * configurations[depth].first;

			// append this value
			my_next_level_configuration.emplace_back(*value_it);

			// explode the next level
			explode_configurations(configurations, my_next_level_configuration, rank_value + rank_value_element, depth + 1);
		}
	}

	void sw_mab_t::define_knobs(learning_configurations_t values)
	{
		// check if there are previous values
		if (!choices.empty())
		{
			choices.clear();
			sliding_window.clear();
			arm_lookup_table.clear();
		}

#ifdef MARGOT_LEARNING_ENABLE_FILE_LOG
		MARGOT_LOG("Learned values: ");

		for ( const auto& value_list : values )
		{
			sw_mab_t::outfile << "\t";

			for ( const auto& value : value_list.second)
			{
				sw_mab_t::outfile << value << " ";
			}

			sw_mab_t::outfile << std::endl;
		}

#endif

		// explode the results (if any value)
		if (!values.empty())
			explode_configurations(values, configuration_t{});

		// normalize the rank value to match the values from the state
		const float max_range_observed = max_rank_value - min_rank_value;

		if (max_range_observed > 0)
		{
			// we can normalize all the values
			for ( auto& arm : choices)
			{
				// compute the new rank value
				const float new_rank_value = (arm->get_arm_rank() - min_rank_value) / max_range_observed * state_t::reward_max_value;
				MARGOT_LOG("Normalized the rank " << arm->get_arm_rank() << " to value " << new_rank_value );

				// set the new rank value for the arm
				arm->set_arm_rank(new_rank_value);
			}
		}
		else
		{
			// all the configurations weight the same, we should reset them to zero to avoid problems
			for ( auto& arm : choices)
			{
				arm->set_arm_rank(0.0f);
			}
		}

		MARGOT_LOG("Generated " << choices.size() << " arms");
		MARGOT_LOG(std::endl << std::endl);
	}

	void sw_mab_t::push_reward( const configuration_t& configuration, const float& reward )
	{
		// get the index of the related arm
		const std::size_t arm_index = arm_lookup_table.at(configuration);

		// check if the configuration is valid
		const bool valid = reward >= state_t::reward_max_value * state_t::reward_balance_coefficiet;

		// get the rank of the arm
		const float arm_rank = choices[arm_index]->get_arm_rank();
		const float real_reward = valid ? reward_balance_coefficiet_arm_state * reward + (1 - reward_balance_coefficiet_arm_state) * arm_rank : reward * reward_balance_coefficiet_arm_state;

		// push the reward in the correct arm
		choices[arm_index]->push_reward(real_reward);

		// update the history on the window
		sliding_window.emplace_back(arm_index);

		MARGOT_LOG("Stored reward \"" << real_reward << "\" for the arm " << arm_index + 1 << " [WINDOW_SIZE: " << sliding_window.size() << "/" << window_size << "]");

		// check if the window is full
		if (sliding_window.size() > window_size)
		{
			// get the last arm used
			const std::size_t last_arm = sliding_window.front();

			// pop the value from the history
			sliding_window.pop_front();

			// also eliminate the reward value from the corresponfing arm
			choices[last_arm]->discard_last_value();

			MARGOT_LOG("\tNeed to eliminate the last value of arm " << arm_index + 1);
		}

		MARGOT_LOG(std::endl << std::endl);
	}

	configuration_t sw_mab_t::get_best_configuration( void )
	{
		// find the arm with greater reward
		float max_reward = std::numeric_limits<float>::min();
		configuration_t configuration;
		MARGOT_LOG("Requested to retrieve the best configuration");
#ifdef MARGOT_LEARNING_ENABLE_FILE_LOG
		std::size_t counter = 0;
#endif


		for ( auto& arm : choices )
		{
			// get the reward
			MARGOT_LOG("\tEvaluating Arm " << ++counter - 1 );
			const float arm_reward = arm->get_expected_reward(sliding_window.size());

			// check if it is the minimum
			if (arm_reward >= max_reward)
			{
				configuration = arm->get_configuration();
				max_reward = arm_reward;
				MARGOT_LOG("\t\tIs the new best!");
			}

			MARGOT_LOG(std::endl);
		}

		MARGOT_LOG(std::endl << std::endl);

		// return the best configuration
		return configuration;
	}


	void sw_mab_t::arm_t::push_reward(const float reward)
	{
		reward_history.emplace_back(reward);
	}

	void sw_mab_t::arm_t::discard_last_value( void )
	{
		reward_history.pop_front();
	}

	configuration_t sw_mab_t::arm_t::get_configuration( void ) const
	{
		return configuration;
	}


	float sw_mab_t::arm_t::get_expected_reward( const std::size_t history_size ) const
	{
		// corner case: no reward, fix to the maximum
		if (reward_history.empty())
		{
			MARGOT_LOG("\t\t\tReward            : " << initializing_reward << " [NO PREVIOUS REWARDS]");
			return initializing_reward;
		}

		// compute the expected reward
		const std::size_t nt = reward_history.size();
		float summatory_reward = 0.0f;

		for ( auto reward : reward_history)
		{
			summatory_reward += reward;
		}

		const float expected_reward = summatory_reward / nt;


		// compute the uncertainty factor
		const float inner_product = uncertainty_coefficient * std::log(history_size) / nt;
		const float uncertainty = state_t::reward_max_value * std::sqrt(inner_product);

		// combine the two factors
		const float total_expected_reward = std::min(initializing_reward, expected_reward + uncertainty );

		MARGOT_LOG("\t\t\tReward factor     : " << expected_reward);
		MARGOT_LOG("\t\t\tUncertainty factor: " << uncertainty);
		MARGOT_LOG("\t\t\tReward            : " << expected_reward + uncertainty << " [WITH " << nt << " VALUES]");

		// combine the two factors
		return total_expected_reward;
	}




}
