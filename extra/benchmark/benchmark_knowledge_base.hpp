#ifndef HEADER_ARGO_BENCHMARK_KB_H
#define HEADER_ARGO_BENCHMARK_KB_H

#include <random>

#include <margot/knowledge_base.hpp>
#include <margot/operating_point.hpp>

#include "bench_base.hpp"


unsigned long int bench_knowledge_creation( margot::operating_points_t& ops)
{

	margot::knowledge_base_t kb;
	chronometer_t c;

	c.start();
	kb.add_operating_points(ops);
	return c.stop();
}



unsigned long int bench_view_creation( margot::operating_points_t& ops )
{

	margot::knowledge_base_t kb;
	chronometer_t c;

	// add the operating points
	kb.add_operating_points(ops);

	// measure the time to create a view
	c.start();
	const auto useless = kb.get_metric_view(3); // on a metric random
	return c.stop();
}


unsigned long int bench_add_op( margot::operating_points_t& ops )
{
	constexpr std::size_t num_initial_op = 100;
	const std::size_t num_ops = ops.size();
	margot::knowledge_base_t kb;
	chronometer_t c;


	// add the initial op_list
	margot::operating_points_t points_temp;
	std::default_random_engine generator;
	std::uniform_int_distribution<int> rnd(1, 100);

	for ( std::size_t i = 0; i < num_initial_op; ++i)
	{
		points_temp.push_back(
		{
			{static_cast<margot::parameter_t>(num_ops + i), static_cast<margot::parameter_t>(num_ops + i + 1), static_cast<margot::parameter_t>(num_ops + i + 2)},
			{static_cast<margot::metric_t>(5), static_cast<margot::metric_t>(num_ops + i), static_cast<margot::metric_t>(num_ops + num_initial_op - 1), static_cast<margot::metric_t>(rnd(generator))}
		});
	}

	kb.add_operating_points(points_temp);


	// create 3 views
	const auto useless1 = kb.get_metric_view(0);
	const auto useless2 = kb.get_metric_view(1);
	const auto useless3 = kb.get_metric_view(2);


	// add the operating points
	c.start();
	kb.add_operating_points(ops);
	return c.stop();
}





unsigned long int bench_remove_op( margot::operating_points_t& ops )
{
	margot::knowledge_base_t kb;
	chronometer_t c;


	// add the initial op_list
	kb.add_operating_points(ops);


	// create 3 views
	const auto useless1 = kb.get_metric_view(0);
	const auto useless2 = kb.get_metric_view(1);
	const auto useless3 = kb.get_metric_view(2);


	// add the operating points
	c.start();
	kb.remove_operating_points(ops);
	return c.stop();
}













#endif
