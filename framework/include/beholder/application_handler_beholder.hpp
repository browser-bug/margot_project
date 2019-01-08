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
#include <sys/stat.h> // to create directories, only for linux systems

#include "agora/virtual_io.hpp"
#include "agora/common_objects.hpp"
#include "beholder/ici_test_data.hpp"
#include "beholder/observation_data.hpp"
#include "beholder/parameters_beholder.hpp"
#include "beholder/common_objects_beholder.hpp"


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
    COMPUTING,
    DISABLED,
    TRAINING,
    RETRAINING
  };

  class RemoteApplicationHandler
  {


    private:

      // to protect the data structure
      std::mutex mutex;

      // suffix counter for plot file
      int suffix_plot;

      // counter of the total observations received (reset at every retraining - reset of ici test).
      // When it is reset after reset if ici test it gets re-initialized at window_size*training_windows values.
      // Does not take into account blacklisted clients or observations arrived while the handler was busy (status==COMPUTING)
      int current_test_observations_counter;

      // counter of the total number of observations analyzed by this handler (never reset)
      // Does not take into account blacklisted clients or observations arrived while the handler was busy (status==COMPUTING)
      int observations_counter;

      // counter of the number of times the re-training has been issued (never reset);
      // It measures the total number of times the ici test was right in detecting a change.
      int retraining_counter;

      // counter of the number of times the ici test has been reset (never reset)
      // It measures the number of times the ici test has been rejected by the 2nd leve test
      int ici_reset_counter;

      // application-specific root workspace
      std::string application_workspace;

      // output files to plot the ICI CDT curves
      // the structure maps each metric name (key) to a struct, whose first element "observations" is the file
      // containing all the observations collected and used in the ICI CDT, the second file "ici"
      // contains the ICI for the mean and for the variance of every window of the CDT.
      std::unordered_map<std::string, output_files> output_files_map;

      // to handle the progress of the elaboration
      ApplicationStatus status;

      // to save the application state when an handler is PAUSED
      ApplicationStatus previous_status;

      // these are the data structures that actually have information
      // about the application behavior
      application_description_t description;

      // Prefix to log strings containing the application name
      std::string log_prefix;

      // clients blacklist, implemented as an unordered set since we do not need any sorting
      std::unordered_set<std::string> clients_blacklist;

      // hash-map storing the list of clients encountered as of now by this handler.
      // the client-id is the hash key, while the value is a struct representing the timestamp
      // The first field of the map is the "seconds", while the second field represents the "nanoseconds".
      std::unordered_map<std::string, timestamp_fields> clients_list;


      // set containing the metrics observed by the beholder
      // NB: these are not all the available metrics, or all the ENABLED metrics,
      // but are instead the metrics the the user explicitely set to be monitored
      // by the beholder in the XML config file.
      // This is to say that it could be the same as all the currently enabled metrics,
      // but it also may be different, according to the user settings.
      std::set<std::string> reference_metric_names;

      // data structure to store the residuals from the observations received,
      // i.e. the difference between the predicted value by the model and the actual value.
      // This structure maps every metric (name) to a vector of structs.
      // The vector of structs is the buffer of residuals for the metric of interested.
      // The buffer will be as big as the beholder parameter "window_size" instructs.
      // The struct's first element is the residual value itself.
      // The struct's second element is a another struct containing the timestamp of the corresponding residual,
      // it has two members: two strings containing respectively the seconds since epoch and nanoseconds.
      // The timestamp of the first and last element in the current window allows us to pinpoint
      // the change detection time range later on in Cassandra's trace
      // We could have different windows for every metric observed, this is the reason why the
      // timestamp of the first and last element are not unique across the whole application handler.
      std::unordered_map<std::string, std::vector<residual_struct>> residuals_map;

      // ICI CDT data structures:
      // It maps every metric to its struct of data for the ICI CDT
      std::unordered_map<std::string, Data_ici_test> ici_cdt_map;

      // Window containing the timestamps (in Ctime's format) about
      // the first and last element of the change window.
      window_timestamps change_window_timestamps;

      std::string change_metric_name;

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

      inline bool create_folder( const std::string& path )
      {
        int rc = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
        return rc == 0 || errno == EEXIST;
      }

      int parse_observation(Observation_data& observation, const std::string& values);
      int fill_buffers(const Observation_data& observation);
      void first_level_test( void );
      void parse_and_insert_observations_for_client_from_trace(std::unordered_map<std::string, residuals_from_trace>& client_residuals_map, const observation_t j,
          const std::set<std::string>& metric_to_be_analyzed);
      void second_level_test( std::unordered_map<std::string, timestamp_fields>& clients_list_snapshot );

      inline void retraining ( void )
      {
        // need to trigger RE-training
        // this automatically deals with the deletion of the model and of the trace and with the reset of the doe
        // in this version the trace is deleted just from the top element in the table up to the last element of the training window
        if (Parameters_beholder::no_trace_drop)
        {
          agora::pedantic(log_prefix, "Deleting the model, restoring the DOE, deleting just the rows of the trace which are before the detected change window.");
          send_agora_command("retraining " + std::to_string(change_window_timestamps.back.seconds) + "," + std::to_string(change_window_timestamps.back.nanoseconds));
        }
        else
        {
          agora::pedantic(log_prefix, "Deleting the model, restoring the DOE, deleting the whole trace.");
          // delete the whole trace
          send_agora_command("retraining");
        }

        retraining_counter++;
        status = ApplicationStatus::TRAINING;
      }



    public:

      // re-activate the beholder handler after a re-training has been issued
      inline void set_handler_ready( void )
      {
        // lock the mutex to ensure a consistent global state
        std::unique_lock<std::mutex> guard(mutex);

        // only when the current status is TRAINING
        if (status == ApplicationStatus::TRAINING)
        {
          agora::info(log_prefix, "Handler put-on-ready after training complete following a re-training request. A new model has arrived!");
          status = ApplicationStatus::READY;
        }
      }

      // pause handler
      inline void pause_handler ( void )
      {
        // lock the mutex to ensure a consistent global state
        std::unique_lock<std::mutex> guard(mutex);

        // only if the current status is not already DISABLED
        if (status != ApplicationStatus::DISABLED)
        {
          agora::info(log_prefix, "Handler put-on-hold after agorà's' kia. Waiting for agorà's resurrection...");
          previous_status = status;
          status = ApplicationStatus::DISABLED;
        }
      }

      // un_pause handler
      inline void un_pause_handler ( void )
      {
        // lock the mutex to ensure a consistent global state
        std::unique_lock<std::mutex> guard(mutex);

        // only if the current status is DISABLED
        if (status == ApplicationStatus::DISABLED)
        {
          agora::info(log_prefix, "Handler re-enabled after agorà's resurrection. Restored previous status.");
          status = previous_status;

          if (status == ApplicationStatus::RETRAINING)
          {
            agora::info(log_prefix, "Resetting the whole application handler after having triggered the re-training following handler un-pause!");
            // handle the retraining
            retraining();
          }
        }
      }


      // removes the current client from the list of active clients encountered by the beholder
      inline void bye_client( const std::string& client_id )
      {
        // lock the mutex to ensure a consistent global state
        std::unique_lock<std::mutex> guard(mutex);
        auto search_client = clients_list.find(client_id);

        if (search_client != clients_list.end())
        {
          clients_list.erase(search_client);
        }
      }

      RemoteApplicationHandler( const std::string& application_name );

      void new_observation( const std::string& values );

  };

}

#endif // MARGOT_BEHOLDER_APPLICATION_HANDLER_HDR
