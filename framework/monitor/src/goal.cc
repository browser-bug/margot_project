/* core/monitor
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

#include <cmath>

#include "margot/goal.hpp"

namespace margot
{

	goal_t::goal_t( void ) {}


	margot::goal_t::goal_t(const margot::ComparisonFunction& c_fun, const statistical_properties_t& value): data(new struct target_t())
	{

		// set the goal value
		data->value = value;

		// set the comparison function correctly
		switch (c_fun)
		{
			case (ComparisonFunction::Greater):
				data->compare = [] (const statistical_properties_t& value1, const statistical_properties_t& value2)
				{
					return value1 > value2;
				};

				break;

			case (ComparisonFunction::GreaterOrEqual):
				data->compare = [] (const statistical_properties_t& value1, const statistical_properties_t& value2)
				{
					return value1 >= value2;
				};

				break;

			case (ComparisonFunction::Less):
				data->compare = [] (const statistical_properties_t& value1, const statistical_properties_t& value2)
				{
					return value1 < value2;
				};

				break;

			case (ComparisonFunction::LessOrEqual):
				data->compare = [] (const statistical_properties_t& value1, const statistical_properties_t& value2)
				{
					return value1 <= value2;
				};

				break;

			default:
				throw std::logic_error("Goal Error: Unable to understand the ComparisonFunction ");
		}
	}


	margot::goal_t::goal_t(std::function<bool(statistical_properties_t&)> target_function,  const ComparisonFunction& c_fun, const statistical_properties_t& goal_value): goal_t(c_fun, goal_value)
	{

		// set the monitor pointer to a null value
		data->monitor_ptr = nullptr;

		// copy the target_function
		data->observed_value = target_function;
	}






	bool margot::goal_t::check()
	{
		statistical_properties_t result = 0;
		bool valid = data->observed_value(result);

		if (!valid && data->compare(result, data->value))
		{
			return true;
		}

		return data->compare(result, data->value);
	}


	statistical_properties_t margot::goal_t::absolute_error()
	{
		// get the observed value
		statistical_properties_t observed_value;
		data->observed_value(observed_value);

		// if it is ok, skip it
		if (data->compare(observed_value, data->value))
		{
			return static_cast<statistical_properties_t>(0);
		}

		// otherwise compute error
		return std::abs(observed_value - data->value);
	}


	statistical_properties_t margot::goal_t::relative_error()
	{
		statistical_properties_t difference = absolute_error();

		if (data->value == 0)
		{
			return difference;
		}
		else
		{
			return difference / data->value;
		}
	}

	statistical_properties_t margot::goal_t::nap()
	{
		// get the observed value
		statistical_properties_t observed_value;
		data->observed_value(observed_value);

		// compute the difference
		statistical_properties_t difference = std::abs(observed_value - data->value);
		statistical_properties_t summ = observed_value + data->value;

		// compute the nap
		if (summ == 0)
		{
			return difference;
		}
		else
		{
			return difference / summ;
		}
	}
}
