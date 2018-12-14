/* beholder/application_handler_beholder.hpp
 * Copyright (C) 2018 Alberto Bendin
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


#ifndef MARGOT_BEHOLDER_APPLICATION_HANDLER_HDR
#define MARGOT_BEHOLDER_APPLICATION_HANDLER_HDR

#include <string>
#include <mutex>
#include <unordered_set>
#include <unordered_map>
#include <cstdint>
#include <cassert>
#include <set>

#include "agora/virtual_io.hpp"
#include "agora/common_objects.hpp"
#include "beholder/ici_test_data.hpp"
#include "beholder/observation_data.hpp"


namespace beholder
{

  using application_description_t = agora::application_description_t;
  using application_list_t = agora::application_list_t;
  using observation_t = std::string;
  using observations_list_t = agora::observations_list_t;

  // for the beholder we are just interested in knowing whether the application has the model or not
  // the beholder will start only when the application applies the model received from agorà
  enum class ApplicationStatus : uint_fast8_t
  {
    READY,
    WITH_MODEL,
    COMPUTING
  };




  class RemoteApplicationHandler
  {


    private:

      // to protect the data structure
      std::mutex mutex;

      // to handle the progress of the elaboration
      ApplicationStatus status;

      // these are the data structures that actually have information
      // about the application behavior
      application_description_t description;

      // Prefix to log strings containing the application name
      std::string log_prefix;

      // clients blacklist, implemented as an unordered set since we do not need any sorting
      std::unordered_set<std::string> clients_blacklist;

      // set containing the metrics observed by the beholder
      // NB: these are not all the available metrics, or all the ENABLED metrics,
      // but are instead the metrics the the user explicitely set to be monitored
      // by the beholder in the XML config file.
      // This is to say that it could be the same as all the currently enabled metrics,
      // but it also may be different, according to the user settings.
      std::set<std::string> reference_metric_names;

      // data structure to store the residuals from the observations received,
      // i.e. the difference between the predicted value by the model and the actual value.
      // This structure maps every metric (name) to a pair.
      // The pair's first element is the buffer of residuals for the metric of interested.
      // The buffer will be as big as the beholder parameter "window_size" instructs.
      // The pair's second element is a string containing the timestamp of the corresponding residual.
      // The timestamp of the first and last element in the current window allows us to pinpoint
      // the change detection time range later on in Cassandra's trace
      // We could have different windows for every metric observed, this is the reason why the
      // timestamp of the first and last element are not unique across the whole application handler.
      std::unordered_map<std::string, std::vector<std::pair <float, std::string>>> residuals_map;

      // TODO: do we really need this structure?
      // data structure to store the counter of the received residuals for each meatric so far
      // useful to understand when the training phase is finished for each metrics
      // considering the case in which not all observations contain residuals for every metric
      // thus leaving us with some metrics which already have completed the training and others
      // which are still gathering observations
      std::unordered_map<std::string, int> residuals_map_counter;

      // ICI CDT data structures:
      // It maps every metric to its struct of data for the CDT
      std::unordered_map<std::string, Data_ici_test> ici_cdt_map;

      // these are the function used to communicate using MQTT topics

      // send a command to all the clients running the application
      inline void send_margot_command( const std::string& command )
      {
        // send the command
        agora::io::remote.send_message({{"margot/" + description.application_name + "/commands"}, command});
      }

      // send an application-specific command to agora
      inline void send_agora_command( const std::string& command )
      {
        // send the command
        agora::io::remote.send_message({{"agora/" + description.application_name + "/commands"}, command});
      }

      void parse_observation(Observation_data& observation, const std::string& values);
      void fill_buffers(const Observation_data& observation);
      void parse_and_insert_observations_for_client_from_trace(std::unordered_map<std::string, std::pair < std::vector<float>, std::vector<float>>>& client_residuals_map, const observation_t j,
          const std::set<std::string>& metric_to_be_analyzed);


    public:

      RemoteApplicationHandler( const std::string& application_name );

      int new_observation( const std::string& values );

  };

}

#endif // MARGOT_BEHOLDER_APPLICATION_HANDLER_HDR
