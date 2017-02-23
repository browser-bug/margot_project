#ifndef HEADER_ARGO_BENCH_BASE_H
#define HEADER_ARGO_BENCH_BASE_H

#include <chrono>
#include <ratio>
#include <string>
#include <cstddef>
#include <vector>
#include <random>
#include <functional>
#include <iostream>


#include <margot/operating_point.hpp>



#include "file_logger.hpp"





typedef struct chronometer_t
{
	std::chrono::steady_clock::time_point start_point;

	inline void start( void )
	{
		start_point = std::chrono::steady_clock::now();
	}

	inline unsigned long int stop( void )
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - start_point).count();
	}
} chronometer_t;



class benchmark_t
{
	public:

		benchmark_t( void ): benchmark_name("default_benchmark"), num_trials(10)
		{
			setup();
		}
		benchmark_t( std::string benchmark_name, std::size_t num_trials = 10 ): benchmark_name(benchmark_name), num_trials(num_trials)
		{
			setup();
		}
		benchmark_t( std::string benchmark_name, std::vector<std::size_t> sizes, std::size_t num_trials = 10 ): benchmark_name(benchmark_name), num_trials(num_trials)
		{
			setup(std::move(sizes));
		}

		// create the Operating Points list
		void setup( std::vector<std::size_t> sizes = {10, 20, 40, 50, 100, 200, 500, 1000})
		{

			// populate the list of Operating Points
			std::default_random_engine generator;
			std::uniform_int_distribution<int> rnd(1, 100);

			for ( const auto size : sizes )
			{
				margot::operating_points_t points_temp;

				for ( std::size_t i = 0; i < size; ++i)
				{
					points_temp.push_back(
					{
						{static_cast<margot::parameter_t>(i), static_cast<margot::parameter_t>(i + 1), static_cast<margot::parameter_t>(i + 2)},
						{static_cast<margot::metric_t>(5), static_cast<margot::metric_t>(i), static_cast<margot::metric_t>(size - 1), static_cast<margot::metric_t>(rnd(generator))}
					});
				}

				points.emplace_back(std::move(points_temp));
			}
		}


		// run the benchmark
		void operator()( std::function<unsigned long int(margot::operating_points_t&)> test) const
		{
			// open the logger file
			margot::logger_t logger;
			logger.open(std::string("result." + benchmark_name + ".csv"), margot::Format::CSV, "Number_ops", "Overhead_us");
			std::cout << "[ " << benchmark_name << " ] ..." << std::flush;


			// loop over the op sizes
			for (auto ops : points)
			{
				// get the number of ops
				const auto evaluated_size = ops.size();

				// repeat the experiment for a certain amount of time
				for (std::size_t trial = 0; trial < num_trials + 5; ++trial)
				{
					// get the overhead
					const auto overhead = test(ops);

					// log the result (skipping first results)
					if (trial > 5)
					{
						logger.write(evaluated_size, overhead);
					}
				}
			}

			std::cout << "done!" << std::endl << std::flush;
		}

	private:

		std::string benchmark_name;

		std::size_t num_trials;

		std::vector<margot::operating_points_t> points;
};


#endif
