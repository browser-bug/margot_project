#ifndef MARGOT_BENCHMARK_EVALUATOR_HDR
#define MARGOT_BENCHMARK_EVALUATOR_HDR

#include <chrono>
#include <vector>
#include <string>
#include <memory>

#include "result_printer.hpp"





template< class TimeType = std::chrono::microseconds >
class Launcher
{

  private:

    std::vector< data_serie_t > results;
    std::vector< int > number_of_ops;
    const std::string experiment_name;

  public:

    Launcher( const std::string& experiment_name ): experiment_name(experiment_name)
    {
      number_of_ops = {10, 20, 30, 50, 100, 200, 300, 500};
    }

    ~Launcher( void )
    {
      plot<TimeType>( experiment_name, results );
    }

    Launcher( const Launcher& ) = delete;
    Launcher( Launcher&& ) = delete;

    inline void set_number_of_ops( std::vector< int > ops )
    {
      number_of_ops = std::move(ops);
    }


    template< class T >
    void run( const std::string& data_serie_name = "", const int number_of_runs = 200 )
    {
      // build the experiment record
      data_serie_t experiment_log;
      experiment_log.name = data_serie_name;


      // run the experiment n times, taking into account cache effects
      const int number_cache_runs = 200;
      const int total_number_of_runs = number_of_runs + number_cache_runs;

      for ( const int num_ops : number_of_ops )
      {
        for (int i = 0; i < total_number_of_runs; ++i)
        {
          // initialize the experiment
          T experiment(num_ops);

          // take the time of now
          const auto starting_point = std::chrono::steady_clock::now();

          // run the experiment
          experiment();

          // compute the elapsed time
          const auto stopping_point = std::chrono::steady_clock::now();
          const uint64_t elapsed_time = std::chrono::duration_cast<std::chrono::nanoseconds>(stopping_point - starting_point).count();

          // store the value if needed
          if (i >= number_cache_runs)
          {
            experiment_log.data.emplace_back(num_ops, elapsed_time);
          }
        }
      }

      // append the result
      results.emplace_back(experiment_log);
    }

};








#endif // MARGOT_BENCHMARK_EVALUATOR_HDR
