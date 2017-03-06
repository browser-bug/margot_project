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
#include <string>


#include <margot/asrtm.hpp>
#include <margot/operating_point.hpp>


namespace margot
{

	asrtm_t::asrtm_t( void ): manager(new asrtm_internal_t())
	{}









#ifdef MARGOT_LEARNING_ENABLE_STATE
	asrtm_t::asrtm_internal_t::asrtm_internal_t( void ): explored_portion_configuration_size(0)
#else
	asrtm_t::asrtm_internal_t::asrtm_internal_t( void )
#endif
	{
		// create a default state
		states["default"] = asrtm_state_t{};

		// update the current state pointer
		current_state = states.begin();

		// set the knowledge
		current_state->second.explored_state.set_knowledge_base(knowledge);

		// set the flag on structure changed
		structure_changed = true;
		state_updated = false;
		removed_current_operating_point = false;

		// set the state
		internal_state = AsrtmState::Empty;
	}



	/****************************************************
	 * Learning definition methods
	 ****************************************************/

	void asrtm_t::asrtm_internal_t::define_learning_sw_ucb_parameters( const learning_state_t::learning_configurations_t           software_knobs,
	        const std::size_t window_size,
	        const float uncertainty_coefficient,
	        const float reward_balance_coef )
	{
#ifdef MARGOT_LEARNING_ENABLE_STATE
		// Create a new learning state
		current_state->second.learning_state.reset(new sw_mab_t(window_size, uncertainty_coefficient, reward_balance_coef));

		// define the parameters
		current_state->second.learning_state->define_knobs(software_knobs);
#endif
	}







	/****************************************************
	 * State manipulation methods
	 ****************************************************/

	void asrtm_t::asrtm_internal_t::add_state(const std::string state_name)
	{
		std::lock_guard<std::mutex> lock(asrtm_mutex);

		// check if the state it is already created
		if (pedantic_check())
		{
			const auto ref_new_state = states.find(state_name);

			if (ref_new_state != states.end())
			{
				throw std::runtime_error("[asrtm_internal_t] Error: attempting to create an already existent state '" + state_name + "'");
			}
		}

		// create the new state
		asrtm_state_t new_state;

		// assign the knowledge
		new_state.explored_state.set_knowledge_base(knowledge);

		// insert the new element
		states.emplace(state_name, std::move(new_state));
	}


	void asrtm_t::asrtm_internal_t::change_active_state(const std::string new_state_name)
	{
		std::lock_guard<std::mutex> lock(asrtm_mutex);

		// look for the new active state
		auto new_state_reference = states.find(new_state_name);

		// check if the state actually exists
		if (pedantic_check())
		{
			if (new_state_reference == states.end())
			{
				throw std::runtime_error("[asrtm_internal_t] Error: unable to switch to a new state, state '" + new_state_name + "' not found'");
			}
		}

		// assign the reference to the current state
		current_state = new_state_reference;

		// issue a synch
		current_state->second.explored_state.synch();

		// signal that the structure is changed
		structure_changed = true;
	}

	void asrtm_t::asrtm_internal_t::remove_state(const std::string state_name)
	{
		std::lock_guard<std::mutex> lock(asrtm_mutex);

		// get a reference to the target state
		auto removed_state = states.find(state_name);

		// perform addition test
		if (pedantic_check())
		{
			// check if we found the element
			if (removed_state == states.end())
			{
				throw std::runtime_error("[asrtm_internal_t] Error: unable to remove a state, state '" + state_name + "' not found");
			}

			// check if it is the active state
			if (removed_state == current_state)
			{
				throw std::runtime_error("[asrtm_internal_t] Error: unable to remove a state, state '" + state_name + "' active");
			}
		}

		// remove the state
		states.erase(removed_state);
	}







	/****************************************************
	 * Operating Points manipulation methods
	 ****************************************************/

	void asrtm_t::asrtm_internal_t::find_best_configuration_internal(  const bool state_changed )
	{
#ifdef MARGOT_LEARNING_ENABLE_STATE

		// check if there is a learning state
		if (!current_state->second.learning_state)
		{
#endif

			// the operation is trivial
			if (state_changed)
			{
				proposed_best_configuration = current_state->second.explored_state.get_best_configuration();
			}

#ifdef MARGOT_LEARNING_ENABLE_STATE
		}
		else
		{
			// we need to get the best configuration from the learning state
			const configuration_t proposed_best_configuration_learning =  current_state->second.learning_state->get_best_configuration();

			// find the error coefficients for the new
			const observation_errors_container_t::const_iterator new_observation_error_it = current_state->second.obervation_errors.find(proposed_best_configuration_learning);

			// if they are found, use them
			if (new_observation_error_it != current_state->second.obervation_errors.cend())
			{
				// save the current observations errors
				const state_t::observation_errors_t actual_observation_error = current_state->second.explored_state.get_observation_errors();

				// pretend to know how the application will behave
				current_state->second.explored_state.set_observation_errors(new_observation_error_it->second);
				current_state->second.explored_state.update(proposed_best_configuration, true);

				// get the best configuration from the explored state
				proposed_best_configuration = current_state->second.explored_state.get_best_configuration();

				// restore the explored state as before
				current_state->second.explored_state.set_observation_errors(actual_observation_error);
				current_state->second.explored_state.update(proposed_best_configuration, true);
			}
			else
			{
				// otherwise suppose that the new arm will the same as the current one
				proposed_best_configuration = current_state->second.explored_state.get_best_configuration();
			}

			// append the learning configuration to the one proposed by the explored state
			proposed_best_configuration.insert(
			    std::end(proposed_best_configuration),
			    std::begin(proposed_best_configuration_learning),
			    std::end(proposed_best_configuration_learning)
			);
		}

#endif // MARGOT_LEARNING_ENABLE_STATE

		if (pedantic_check())
		{
			current_state->second.explored_state.sanity_checks();
		}
	}

	void asrtm_t::asrtm_internal_t::remove_operating_points(operating_points_t& ops)
	{
		std::lock_guard<std::mutex> lock(asrtm_mutex);

		// remove the Operating Points from the knowledge
		knowledge.remove_operating_points(ops);

		// remove the Operating Points from the current state
		current_state->second.explored_state.remove_operating_points(ops);

		// signal that the structure is changed
		structure_changed = true;

		// check if we have removed the current Operating Point
		try
		{
			knowledge.get_operating_point(actual_configuration);
		}
		catch ( std::out_of_range& error )
		{
			removed_current_operating_point = true;
		}

		// check if we have removed the last Operating Point
		if (knowledge.empty())
		{
			// change the internal state to initialized
			internal_state = AsrtmState::Empty;

			// clear all the states
			states.clear();

			// create the new default state
			asrtm_state_t new_state;

			// assign the knowledge
			new_state.explored_state.set_knowledge_base(knowledge);

			// insert the new element
			states.emplace("default", std::move(new_state));

			// clear both the configurations
			proposed_best_configuration.clear();
			actual_configuration.clear();
		}

		if (pedantic_check())
		{
			current_state->second.explored_state.sanity_checks();
		}
	}

	void asrtm_t::asrtm_internal_t::add_operating_points(operating_points_t& ops)
	{
		// get the size of the parameter
		if (!ops.empty())
		{
			std::lock_guard<std::mutex> lock(asrtm_mutex);

#ifdef MARGOT_LEARNING_ENABLE_STATE
			// get the size of the configuration of the first Operating Point
			explored_portion_configuration_size = ops.begin()->first.size();
#endif

			// add the Operating Points to the knowledge
			knowledge.add_operating_points(ops);

			// add the Operating Points to the current state
			current_state->second.explored_state.add_operating_points(ops);

			// signal that the structure is changed
			structure_changed = true;

			// check if it is the first time that we have added some Operating Points
			if ( internal_state == AsrtmState::Empty )
			{

				// find the best configuration for the first time
				find_best_configuration_internal(true);

				// update the state
				internal_state = AsrtmState::Initialized;

				// copy it also for the actual parameters
				actual_configuration = proposed_best_configuration;
			}
		}

		if (pedantic_check())
		{
			current_state->second.explored_state.sanity_checks();
		}
	}




	void asrtm_t::asrtm_internal_t::update( void )
	{
		std::lock_guard<std::mutex> lock(asrtm_mutex);

		// update the state of the autotuner
		if ( internal_state == AsrtmState::Running || internal_state == AsrtmState::NeedAdapt || current_state->second.explored_state.is_not_observable())
		{
#ifdef MARGOT_LEARNING_ENABLE_STATE

			// check if there is a learning state
			if ( current_state->second.learning_state )
			{
				// obtain the configuration from the explored state
				const configuration_t explored_configuration(actual_configuration.begin(), actual_configuration.begin() + explored_portion_configuration_size);

				// update the explored state
				state_updated = current_state->second.explored_state.update(explored_configuration, removed_current_operating_point);

				// check if the state is observable
				if (current_state->second.explored_state.is_observable())
				{
					// get the configuration for the learning state
					const configuration_t learned_configuration(actual_configuration.begin() + explored_portion_configuration_size, actual_configuration.end());

					// make sure to get the best configuration from the explored state
					const configuration_t real_explored_configuration = current_state->second.explored_state.get_best_configuration();

					// get the reward from the managed state
					const float actual_reward = current_state->second.explored_state.get_reward(real_explored_configuration);

					// feed the reward in the learning state
					current_state->second.learning_state->push_reward(learned_configuration, actual_reward);

					// get the observation errors
					const state_t::observation_errors_t actual_observation_error = current_state->second.explored_state.get_observation_errors();

					// store them in the lookup table
					current_state->second.obervation_errors.erase(learned_configuration);
					current_state->second.obervation_errors.emplace(learned_configuration, actual_observation_error);
				}
			}
			else
			{
#endif // MARGOT_LEARNING_ENABLE_STATE
				// update the explored state
				state_updated = current_state->second.explored_state.update(actual_configuration, removed_current_operating_point);
#ifdef MARGOT_LEARNING_ENABLE_STATE
			}

#endif // MARGOT_LEARNING_ENABLE_STATE
		}

		if (pedantic_check())
		{
			current_state->second.explored_state.sanity_checks();
		}
	}

	void asrtm_t::asrtm_internal_t::find_best_operating_point( void )
	{
		std::lock_guard<std::mutex> lock(asrtm_mutex);

		if ( internal_state != AsrtmState::Empty && internal_state != AsrtmState::Configuring )
		{
			// find the best configuration
			find_best_configuration_internal(structure_changed || state_updated);


			// check if they are the same
			if ( proposed_best_configuration == actual_configuration )
			{
				// check for the transition running -> needAdapt
				if ( internal_state == AsrtmState::NeedAdapt )
				{
					internal_state = AsrtmState::Running;
				}
			}
			else
			{
				// check for the transition needAdapt -> running
				if ( internal_state == AsrtmState::Running )
				{
					internal_state = AsrtmState::NeedAdapt;
				}
			}

			// at this point all the changes are being taken care of
			state_updated = false;
			structure_changed = false;
		}
	}


	margot::configuration_t asrtm_t::asrtm_internal_t::get_best_configuration(bool* changed)
	{
		std::lock_guard<std::mutex> lock(asrtm_mutex);

		// handle the update flag
		if (changed != nullptr)
		{
			*changed = internal_state == AsrtmState::Running  || internal_state == AsrtmState::Empty ? false : true;
		}

		// make the transition NeddAdapt -> Configuring
		if ( internal_state == AsrtmState::NeedAdapt )
		{
			internal_state = AsrtmState::Configuring;
		}

		// note: this return an unitialized configuration if the state is empty
		//       however, the flag is setted to false
		return proposed_best_configuration;
	}


	void asrtm_t::asrtm_internal_t::configuration_applied( void )
	{
		std::lock_guard<std::mutex> lock(asrtm_mutex);

		if ( internal_state == AsrtmState::Configuring || internal_state == AsrtmState::Initialized )
		{
			internal_state = AsrtmState::Running;
			actual_configuration = proposed_best_configuration;
			current_state->second.explored_state.clear_monitors();
			removed_current_operating_point = false;
		}
	}

	void asrtm_t::asrtm_internal_t::configuration_rejected( void )
	{
		std::lock_guard<std::mutex> lock(asrtm_mutex);

		if ( internal_state == AsrtmState::Configuring )
		{
			internal_state = AsrtmState::NeedAdapt;
		}
	}













	/****************************************************
	 * State manipulation methods
	 ****************************************************/


	goal_t asrtm_t::create_static_goal_parameter( field_name_t d_fun, ComparisonFunction c_fun, margot_value_t value )
	{
		asrtm_internal_ptr_t dummy_cpy = manager;
		goal_t goal = goal_t( [dummy_cpy, d_fun] (statistical_properties_t& observed_value)
		{
			observed_value = dummy_cpy->get_parameter_value(d_fun);
			return false;
		}, c_fun, value );
		return goal;
	}

	goal_t asrtm_t::create_static_goal_metric( field_name_t d_fun, ComparisonFunction c_fun, margot_value_t value )
	{
		asrtm_internal_ptr_t dummy_cpy = manager;
		goal_t goal = goal_t( [dummy_cpy, d_fun] (statistical_properties_t& observed_value)
		{
			observed_value = dummy_cpy->get_metric_value(d_fun);
			return false;
		}, c_fun, value );
		return goal;
	}

	void asrtm_t::asrtm_internal_t::add_parameter_constraint(goal_t& goal, const field_name_t field, const priority_t priority)
	{
		std::lock_guard<std::mutex> lock(asrtm_mutex);

		if ( internal_state != AsrtmState::Empty )
		{
			// add the constraint
			current_state->second.explored_state.add_parameter_constraint(field, goal, priority);

			// signal that the structure is changed
			structure_changed = true;
		}
	}

	void asrtm_t::asrtm_internal_t::add_metric_constraint( goal_t& goal, const field_name_t field, const priority_t priority )
	{
		std::lock_guard<std::mutex> lock(asrtm_mutex);

		if ( internal_state != AsrtmState::Empty )
		{
			// add the constraint
			current_state->second.explored_state.add_metric_constraint(field, goal, priority);

			// signal that the structure is changed
			structure_changed = true;
		}
	}

	void asrtm_t::asrtm_internal_t::remove_constraint(const priority_t priority)
	{
		std::lock_guard<std::mutex> lock(asrtm_mutex);

		if ( internal_state != AsrtmState::Empty )
		{
			// remove the constraint
			current_state->second.explored_state.remove_constraint(priority);

			// signal that the structure is changed
			structure_changed = true;
		}
	}





	/****************************************************
	 * Accessory methods
	 ****************************************************/


	void asrtm_t::asrtm_internal_t::dump( void ) const
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
		cout << "°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°" << endl;
		cout << "°°°                          ASRTM DUMP                          °°°" << endl;
		cout << "°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°" << endl;


		cout << endl;


		// print the overall status
		cout << "=================================================[GENERAL STATUS]===" << endl;
		cout << "current state:      ";

		switch (static_cast<int>(internal_state))
		{
			case static_cast<int>(AsrtmState::Initialized):
				cout << "AsrtmState::Initialized" << endl;
				break;

			case static_cast<int>(AsrtmState::Running):
				cout << "AsrtmState::Running" << endl;
				break;

			case static_cast<int>(AsrtmState::NeedAdapt):
				cout << "AsrtmState::NeedAdapt" << endl;
				break;

			case static_cast<int>(AsrtmState::Configuring):
				cout << "AsrtmState::Configuring" << endl;
				break;

			default:
				cout << "UNKNOWN" << endl;
				break;
		}

		cout << "available states:   " << states.size() << endl;
		cout << "current state:      " << current_state->first << endl;

		if (structure_changed)
		{
			cout << "structure changed:      TRUE" << endl;
		}
		else
		{
			cout << "structure changed:      FALSE" << endl;
		}

		cout << endl << "PROPOSED BEST CONFIGURATION:" << endl;
		dump_configuration(proposed_best_configuration);
		cout << endl << "ACTUAL CONFIGURATION:" << endl;
		dump_configuration(actual_configuration);


		cout << endl << endl;


		cout << "=================================================[CURRENT STATE]===" << endl;
		current_state->second.explored_state.dump();


		cout << "*****************************END************************************" << endl;
		cout << "°°°°°°°°°°°°°°°°°°°°°°°°°°°°°END°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°" << endl;
	}



}
