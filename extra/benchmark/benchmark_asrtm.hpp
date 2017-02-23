#ifndef HEADER_ARGO_ASRTM_BENCH_H
#define HEADER_ARGO_ASRTM_BENCH_H

#include <random>

#include <margot/asrtm.hpp>

#include "bench_base.hpp"

unsigned long int asrtm_add_ops( margot::operating_points_t& ops)
{
	// declare the chronometer
	chronometer_t c;

	// declare the asrtm
	margot::asrtm_t manager;

	// add the Operating Points
	c.start();
	manager.add_operating_points(ops);
	return c.stop();
}




unsigned long int asrtm_add_constraint_random( margot::operating_points_t& ops)
{
	// declare the chronometer
	chronometer_t c;

	// declare the asrtm
	margot::asrtm_t manager;

	// add the Operating Points
	manager.add_operating_points(ops);


	// get a static goal
	auto goal = manager.create_static_goal_metric(3, margot::ComparisonFunction::Greater, 50);


	// add the constraint
	c.start();
	manager.add_metric_constraint(goal, 3, 10);
	return c.stop();
}



unsigned long int asrtm_add_constraint_equal( margot::operating_points_t& ops)
{
	// declare the chronometer
	chronometer_t c;

	// declare the asrtm
	margot::asrtm_t manager;

	// add the Operating Points
	manager.add_operating_points(ops);


	// get a static goal
	auto goal = manager.create_static_goal_metric(0, margot::ComparisonFunction::Greater, 50);

	// add the constraint
	c.start();
	manager.add_metric_constraint(goal, 0, 10);
	const auto result = c.stop();

	return result;
}



unsigned long int asrtm_add_constraint_scaling( margot::operating_points_t& ops)
{
	// declare the chronometer
	chronometer_t c;

	// declare the asrtm
	margot::asrtm_t manager;

	// add the Operating Points
	manager.add_operating_points(ops);


	// get a static goal
	auto goal = manager.create_static_goal_metric(1, margot::ComparisonFunction::Greater, ops.size() / 2);

	// add the constraint
	c.start();
	manager.add_metric_constraint(goal, 1, 10);
	const auto result = c.stop();

	return result;
}










#endif
