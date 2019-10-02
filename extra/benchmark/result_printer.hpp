#ifndef MARGOT_RESULT_PRINTER_HDR
#define MARGOT_RESULT_PRINTER_HDR

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <fstream>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "margot/statistics.hpp"

struct data_serie_t {
  using value_type = std::pair<int, uint64_t>;

  std::vector<value_type> data;
  std::string name;
};

template <class TimeType = std::chrono::microseconds>
void plot(const std::string& file_basename, const std::vector<data_serie_t>& data_series) {
  // figuring out the suffix of the unit of measures for times
  std::string time_suffix;

  if (std::is_same<TimeType, std::chrono::nanoseconds>::value) {
    time_suffix = "[ns]";
  } else if (std::is_same<TimeType, std::chrono::microseconds>::value) {
    time_suffix = "[us]";
  } else if (std::is_same<TimeType, std::chrono::milliseconds>::value) {
    time_suffix = "[ms]";
  } else if (std::is_same<TimeType, std::chrono::seconds>::value) {
    time_suffix = "[sec]";
  } else if (std::is_same<TimeType, std::chrono::minutes>::value) {
    time_suffix = "[minutes]";
  } else if (std::is_same<TimeType, std::chrono::hours>::value) {
    time_suffix = "[hours]";
  } else {
    time_suffix = "[unkown]";
  }

  // ------------------------------------  write the gnuplot script

  // open the gnuplot file
  std::ofstream plot;
  plot.open(file_basename + ".gnuplot");

  // write the preamble of the gnuplot script
  plot << "reset" << std::endl;
  plot << "set terminal pdf enhanced font 'Verdana,24' size 15,10" << std::endl;
  plot << "unset title" << std::endl;
  plot << "set linestyle 1 lc rgb '#1B9E77' lt 1 lw 12 pt 7 ps 1.5 pi -1   # --- teal" << std::endl;
  plot << "set linestyle 2 lc rgb '#D95F02' lt 1 lw 12 pt 7 ps 1.5 pi -1   # --- orange" << std::endl;
  plot << "set linestyle 3 lc rgb '#7570B3' lt 1 lw 12 pt 7 ps 1.5 pi -1   # --- lilac" << std::endl;
  plot << "set linestyle 4 lc rgb '#E7298A' lt 1 lw 12 pt 7 ps 1.5 pi -1   # --- dark magenta" << std::endl;
  plot << "set grid back" << std::endl;
  plot << "set tics nomirror" << std::endl;
  plot << "set key above" << std::endl;
  plot << "set border 3" << std::endl;
  plot << "set ylabel \"Overhead " << time_suffix << "\"" << std::endl;
  plot << "set xlabel \"Size knowledge base [# Operating Points]\"" << std::endl;
  plot << "set pointintervalbox 3" << std::endl;
  plot << "set style fill transparent solid 0.5 noborder" << std::endl;
  plot << "plot ";

  // write the plot command for the dataseries
  int serie_counter = 0;

  for (const data_serie_t& serie : data_series) {
    const std::string title_bit = serie.name.empty() ? "notitle" : "title \"" + serie.name + "\"";
    // plot << "\"" << file_basename << serie_counter << ".data\" u 1:2:4 with filledcu ls " << serie_counter
    // + 1 << " notitle, ";
    plot << "\"" << file_basename << serie_counter << ".data\" u 1:3 with linespoints ls "
         << serie_counter + 1 << " " << title_bit << ", ";
    ++serie_counter;
  }

  // write the final endline to close the command script
  plot << std::endl;
  plot.close();

  // ------------------------------------  write the data file

  // loop over the data serie
  serie_counter = 0;

  for (const data_serie_t& serie : data_series) {
    // open the datafile
    std::ofstream df;
    df.open(file_basename + std::to_string(serie_counter) + ".data");
    ++serie_counter;

    // print the header
    df << "# num_ops\tminus_sigma\tmean\tplus_sigma" << std::endl;

    // cluster the data according to the number of Operating Points
    std::map<int, std::vector<uint64_t> > clustered_data;

    for (const auto data_pair : serie.data) {
      // insert the data
      const auto it = clustered_data.find(data_pair.first);

      if (it == clustered_data.end()) {
        auto result = clustered_data.emplace(data_pair.first, std::vector<uint64_t>{});
        result.first->second.emplace_back(data_pair.second);
      } else {
        it->second.emplace_back(data_pair.second);
      }
    }

    // loop over the values
    for (const auto pair : clustered_data) {
      // compute the statistics with high precision
      const auto average_value = margot::average<std::vector<uint64_t>, double>(pair.second);
      const auto standard_deviation =
          margot::stddev<std::vector<uint64_t>, double>(pair.second, average_value);

      // compute the upper and lower bound (they are in double, representing ns)
      const auto upper_bound = average_value + standard_deviation;
      const auto average = average_value;
      const auto lower_bound = std::max(static_cast<double>(0), average_value - standard_deviation);

      // compute the conversion ratio in double (for the given time format)
      const uint64_t time_in_nanosec =
          std::chrono::duration_cast<std::chrono::nanoseconds>(TimeType(1)).count();
      const double ratio = static_cast<long double>(1) / static_cast<long double>(time_in_nanosec);

      // convert them in microseconds
      const double actual_upper_bound = upper_bound * ratio;
      const double actual_average = average * ratio;
      const double actual_lower_bound = lower_bound * ratio;

      // log the result
      df << pair.first << '\t' << actual_upper_bound << '\t' << actual_average << '\t' << actual_lower_bound
         << std::endl;
    }

    // close the datafile
    df.close();
  }
}

#endif  // MARGOT_RESULT_PRINTER_HDR
