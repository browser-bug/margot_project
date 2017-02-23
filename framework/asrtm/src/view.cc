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

#include <margot/view.hpp>



namespace margot
{

	view_t::view_t( void ): unique_name(field_name_t{})
	{
		extractor = [] (const operating_points_t::value_type& )
		{
			throw std::runtime_error("[view_t] Error: Undefined extractor (view_t not properly initialized?)");
			return 0;
		};
	}



	view_t::view_t( extractor_t extractor, const field_name_t field, const configuration_map_t& points, const bool is_parameter  ): extractor(extractor), unique_name(is_parameter ? -field : field)
	{
		// popolate the map
		if (is_parameter)
		{
			for ( const auto& point : points )
			{
				view.emplace(static_cast<margot_value_t>(point.first[field]), point.first);
			}
		}
		else
		{
			for ( const auto& point : points )
			{
				view.emplace(static_cast<margot_value_t>(point.second[field]), point.first);
			}
		}
	}

	void view_t::add(const operating_points_t& ops)
	{
		// loop over the ops to be added
		for (const operating_points_t::value_type& op : ops)
		{
			// get its value
			const margot_value_t value = extractor(op);

			// insert the op
			view.emplace(value, op.first);
		}
	}


	void view_t::remove(const operating_points_t& ops)
	{
		// loop over the ops to be removed
		for (const operating_points_t::value_type& op : ops)
		{
			// get its value
			const margot_value_t value = extractor(op);

			// find all the ops with the same value
			const auto equivalent_ops = view.equal_range(value);

			// remove the target one
			for ( auto it = equivalent_ops.first; it != equivalent_ops.second; ++it)
			{
				if (it->second == op.first)
				{
					// delete the element
					view.erase(it);
					break;
				}
			}
		}
	}


	margot_value_t view_t::get_minimum_value( void ) const
	{
		if (view.empty())
		{
			return 0;
		}

		return view.cbegin()->first;
	}

	margot_value_t view_t::get_maximum_value( void ) const
	{
		if (view.empty())
		{
			return 0;
		}

		auto iterator = view.cend();
		std::advance(iterator, -1);
		return iterator->first;
	}


}
