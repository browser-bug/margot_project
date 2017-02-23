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


#include <vector>
#include <deque>
#include <cstddef>
#include <unordered_map>
#include <limits>


#include "margot/config.hpp"
#include "margot/learning_state.hpp"

#ifdef MARGOT_LEARNING_ENABLE_FILE_LOG
	#include <fstream>
#endif // MARGOT_LEARNING_ENABLE_FILE_LOG

#ifndef MARGOT_MAB_SW_UCB_H
#define MARGOT_MAB_SW_UCB_H


/**
 * @brief The namespace for the mARGOt framework
 */
namespace margot
{


	/**
	 * @brief Manage the software knobs that should be learned online
	 *
	 * @details
	 * This class aims at learing at run-time the best configuration of sfotware knobs, which
	 * are not evaluated at design-time through a DSE.
	 * This class is a simple interface, independent from the underlying learning framework.
	 */
	class sw_mab_t: public learning_state_t
	{
			using reward_history_t = std::deque<std::size_t>;
			using arm_map_t = std::unordered_map< configuration_t, std::size_t, configuration_hash_t >;

		public:

			sw_mab_t( const std::size_t window_size = 1000, const float uncertainty_coefficient = 0.5f, const float reward_balance_coefficiet_arm_state = 1.0f):
				window_size(window_size),
				uncertainty_coefficient(uncertainty_coefficient),
				reward_balance_coefficiet_arm_state(reward_balance_coefficiet_arm_state),
				min_rank_value(std::numeric_limits<float>::max()),
				max_rank_value(std::numeric_limits<float>::lowest())
			{
#ifdef MARGOT_LEARNING_ENABLE_FILE_LOG

				if (!sw_mab_t::outfile.is_open())
				{
					sw_mab_t::outfile.open("margot_learning.log");
				}

#endif // LEARNING_ENABLE_FILE_LOG
			}


			void define_knobs( learning_state_t::learning_configurations_t values );
			void push_reward( const configuration_t& configuration, const float& reward );
			configuration_t get_best_configuration( void );

		private:

			void explode_configurations( const learning_configurations_t& configurations,
			                             configuration_t evaluated_configuration,
			                             const float rank_value,
			                             const std::size_t depth  = 0);


			/**
			* @brief This class represents an arm in the context of the multi armed bandit problem
			*/
			class arm_t
			{
				public:
					arm_t( configuration_t configuration, const float uncertainty_coefficient, const float arm_rank ):
						configuration(configuration),
						uncertainty_coefficient(uncertainty_coefficient),
						arm_rank(arm_rank) {}
					void push_reward(const float reward);
					float get_expected_reward( const std::size_t history_size ) const;
					inline float get_arm_rank( void ) const
					{
						return arm_rank;
					}
					inline void set_arm_rank( const float value )
					{
						arm_rank = value;
					}
					void discard_last_value( void );
					configuration_t get_configuration( void ) const;


				private:

					const configuration_t configuration;
					std::deque<float> reward_history;
					const float uncertainty_coefficient;
					float arm_rank;
			};

			using arm_ptr_t = std::unique_ptr<arm_t>;
			using arms_t = std::vector<arm_ptr_t>;



#ifdef MARGOT_LEARNING_ENABLE_FILE_LOG
			static std::ofstream outfile;
#endif // LEARNING_ENABLE_FILE_LOG

			arms_t choices;
			reward_history_t sliding_window;
			arm_map_t arm_lookup_table;
			const std::size_t window_size;
			const float uncertainty_coefficient;
			const float reward_balance_coefficiet_arm_state;
			float min_rank_value;
			float max_rank_value;
	};



}

#endif // MARGOT_MAB_SW_UCB_H
