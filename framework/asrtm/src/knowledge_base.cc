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

#include <margot/knowledge_base.hpp>



namespace margot
{

	knowledge_base_t::knowledge_base_t( void )
	{
		knowledge.reset(new knowledge_t());
	}


	void knowledge_base_t::knowledge_t::add_description(translator_t parameters, translator_t metrics)
	{
		parameter_translator = std::move(parameters);
		metric_translator = std::move(metrics);
	}


	void knowledge_base_t::knowledge_t::add_operating_points(const operating_points_t& ops)
	{
		// loop over the Operating Points
		for ( const operating_points_t::value_type& point : ops )
		{
			// add the the point map
			knowledge.emplace(point.first, point.second);
		}

		// check if it is the first time that are added Operating Points
		if (parameter_views.empty())
		{
			// get the geometry of an Operating Point
			const auto first_op = ops.cbegin();
			const std::size_t num_parameters = first_op->first.size();
			const std::size_t num_metrics = first_op->second.size();

			// resize the vectors
			parameter_views.resize(num_parameters, view_t_ptr(nullptr));
			metric_views.resize(num_metrics, view_t_ptr(nullptr));
		}
		else
		{
			// otherwise check if we need to update the views

			// loop over the parameters view
			for ( const views_t::value_type& view : parameter_views )
			{
				// add the points
				if (view)
				{
					view->add(ops);
				}
			}

			// loop over the metric views
			for ( const views_t::value_type& view : metric_views )
			{
				// add the points
				if (view)
				{
					view->add(ops);
				}
			}
		}

		// update the version of the knowledge
		version = std::chrono::steady_clock::now();
	}



	void knowledge_base_t::knowledge_t::remove_operating_points(const operating_points_t& ops)
	{
		// loop over the Operating Points
		for ( const operating_points_t::value_type& point : ops )
		{
			// add the the point map
			knowledge.erase(point.first);
		}


		// check if we have removed all the Operating Points
		if (knowledge.empty())
		{
			// clear all the views
			parameter_views.clear();
			metric_views.clear();
		}
		else
		{
			// otherwise remove the Operating Points from the view

			// loop over the parameters view
			for ( const views_t::value_type& view : parameter_views )
			{
				// add the points
				if (view)
				{
					view->remove(ops);
				}
			}

			// loop over the metric views
			for ( const views_t::value_type& view : metric_views )
			{
				// add the points
				if (view)
				{
					view->remove(ops);
				}
			}
		}

		// update the version of the knowledge
		version = std::chrono::steady_clock::now();
	}



	view_t_ptr knowledge_base_t::knowledge_t::get_parameter_view(const field_name_t param_name)
	{
		// check if there are Operating Points
		if (pedantic_check())
			if (parameter_views.empty())
			{
				throw std::runtime_error("[knowledge_t] Error: unable to retrieve a view with an empty knowledge");
			}

		// return the reference if it is allocated
		if (parameter_views[param_name])
		{
			return parameter_views[param_name];
		}

		// otherwise create the reference
		parameter_views[param_name].reset(new view_t(view_t::parameter_view(param_name, knowledge)));
		return parameter_views[param_name];
	}



	view_t_ptr knowledge_base_t::knowledge_t::get_metric_view(const field_name_t metric_name)
	{
		// check if there are Operating Points
		if (pedantic_check())
			if (metric_views.empty())
			{
				throw std::runtime_error("[knowledge_t] Error: unable to retrieve a view with an empty knowledge");
			}

		// return the reference if it is allocated
		if (metric_views[metric_name])
		{
			return metric_views[metric_name];
		}

		// otherwise create the reference
		metric_views[metric_name].reset(new view_t(view_t::metric_view(metric_name, knowledge)));
		return metric_views[metric_name];
	}








}
