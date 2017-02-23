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

#ifndef MARGOT_ASRTM_LEARNING_STATE
#define MARGOT_ASRTM_LEARNING_STATE


#include <memory>
#include <vector>
#include <utility>


#include <margot/config.hpp>
#include <margot/operating_point.hpp>
#include <margot/state.hpp>


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
	class learning_state_t
	{
		public:

			using knob_values_t = std::vector<parameter_t>;
			using weighted_values_t = std::pair<float, knob_values_t>;
			using learning_configurations_t = std::vector<weighted_values_t>;

			virtual void define_knobs( learning_configurations_t values ) = 0;
			virtual void push_reward( const configuration_t& configuration, const float& reward ) = 0;
			virtual configuration_t get_best_configuration( void ) = 0;
			virtual ~learning_state_t( void ) {}


		protected:

			learning_state_t( void ) {}
	};


	using learning_state_ptr_t = std::unique_ptr<learning_state_t>;

}


#endif // MARGOT_ASRTM_LEARNING_STATE
