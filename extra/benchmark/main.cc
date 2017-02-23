#include <iostream>
#include <cstdlib>

#include "bench_base.hpp"
#include "benchmark_knowledge_base.hpp"
#include "benchmark_state.hpp"
#include "benchmark_asrtm.hpp"



int main(int argc, char* argv[])
{
	std::cout << "Running framework benchmark:" << std::endl;


	// Perform the benchmark on the kb creation
	benchmark_t b1("knowledge_base_creation");
	b1(bench_knowledge_creation);

	// Perform the benchmark on creating a view
	benchmark_t b2("knowledge_base_view_creation");
	b2(bench_view_creation);

	// Perform the benchmark on adding OPS with three views
	benchmark_t b3("knowledge_base_add_ops_3_views");
	b3(bench_add_op);

	// Perform the benchmark on removing OPS with three views
	benchmark_t b4("knowledge_base_remove_ops_3_views");
	b4(bench_remove_op);




	// Perform the benchmark on computing the linear rank
	benchmark_t b5("state_compute_linear_rank");
	b5(bench_set_linear_rank_3);




	// -----------------------
	// Test the whole asrtm
	// -----------------------

	// Add Operating Points
	// Create a constraint
	benchmark_t b7("asrtm_add_constraint_random");
	b7(asrtm_add_constraint_random);
	benchmark_t b8("asrtm_add_constraint_equal");
	b8(asrtm_add_constraint_equal);
	benchmark_t b9("asrtm_add_constraint_scaling");
	b9(asrtm_add_constraint_scaling);
	benchmark_t b6("asrtm_add_ops");
	b6(asrtm_add_ops);



	std::cout << "All done!" << std::endl;

	return EXIT_SUCCESS;
}
