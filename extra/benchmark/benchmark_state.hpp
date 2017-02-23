#ifndef HEADER_ARGO_STATE_BENCH_H
#define HEADER_ARGO_STATE_BENCH_H

#include <random>

#include <margot/state.hpp>
#include <margot/operating_point.hpp>

#include "bench_base.hpp"


unsigned long int bench_set_linear_rank_3( margot::operating_points_t& ops)
{

	margot::knowledge_base_t kb;
	margot::state_t state;
	chronometer_t c;

	// set the knowledge base
	kb.add_operating_points(ops);
	state.set_knowledge_base(kb);

	c.start();
	state.define_linear_rank(margot::RankObjective::Minimize, margot::rank_metric_t{0, 1.0},
	                         margot::rank_metric_t{1, 2.0},
	                         margot::rank_metric_t{2, 3.0});
	return c.stop();
}













#endif
