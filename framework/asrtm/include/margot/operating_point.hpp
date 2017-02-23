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


#ifndef MARGOT_ASRTM_OPERATING_POINT_HEADER
#define MARGOT_ASRTM_OPERATING_POINT_HEADER



#include <deque>
#include <unordered_set>
#include <utility>
#include <vector>
#include <map>


#include <margot/config.hpp>


/**
 * @brief The namespace for the mARGOt framework
 */
namespace margot
{

	/**
	 * @brief Represents the name of an enum
	 *
	 * @details
	 * This class is meant to be extended by a class the actually
	 * implements
	 */

	/**
	 * @brief The type of the key used to identify a parameter/metric
	 */
	using field_name_t = std::size_t;

	/**
	 * @brief The type of a parameter of the configuration
	 */
	using parameter_t = margot_value_t;


	/**
	 * @brief The type of a metric of the performance
	 */
	using metric_t = margot_value_t;

	/**
	 * @brief The list of parameters
	 */
	using configuration_t = std::vector< parameter_t >;


	/**
	 * @brief The list of metrics
	 */
	using performance_t = std::vector< metric_t >;


	/**
	 * @brief hash function for a configuration
	 *
	 * @details
	 * This struct is meant to be used to compute the hash of vector of integer
	 * in a fast way
	 */
	struct configuration_hash_t
	{
		size_t operator()(const configuration_t& v) const
		{
			std::hash<configuration_t::value_type> hasher;
			size_t seed = 0;

			for (const configuration_t::value_type i : v)
			{
				seed ^= hasher(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
			}

			return seed;
		}
	};

	/**
	 * @brief The Configuration look-up table
	 *
	 * @details
	 * This structure it is used as a fast look-up table for the configurations.
	 * It is not efficient to iterate through the structure, however it is
	 * efficient to add/remove/look-up configurations
	 */
	using lookup_table_t = std::unordered_set< configuration_t, configuration_hash_t >;


	/**
	 * @brief The definition of an Operating Point
	 */
	using operating_point_t = std::pair< configuration_t, performance_t >;



	/**
	 * @brief The list of Operating Points
	 *
	 * @details
	 * It is meant to store a defined set of Operating Point. Iteration and random
	 * deletes are efficient operations.
	 */
	using operating_points_t = std::deque< operating_point_t >;


	/**
	 * @brief The list of configurations
	 *
	 * @details
	 * It is meant to store a defined set of configurations. Iteration and random
	 * deletes are efficient operations.
	 */
	using configurations_t = std::deque< configuration_t >;


	/**
	 * @brief The configuration map that relates a configuration with its performance
	 */
	using configuration_map_t = std::map< configuration_t, performance_t >;
}

#endif // MARGOT_ASRTM_OPERATING_POINT_HEADER
