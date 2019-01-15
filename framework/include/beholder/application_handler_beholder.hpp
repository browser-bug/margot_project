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

  /**
   * @brief Internal state of the ApplicationHandler
   */
  enum class ApplicationStatus : uint_fast8_t
  {
    /**
     * @details
     * In this state the ApplicationHandler is ready to receive new observations used for the ici test.
     */
    READY,

    /**
     * @details
     * In this state the ApplicationHandler is rejecting any incoming observation because it
     * is currently working on the 2nd level of the change detection test, i.e. the hypothesis test.
     * The lock is released because the interaction with the storage can be slow, and in this way
     * the incoming observations are processed in any case (be discarded).
     * This is also the state in which the handler remains if while working on the 2nd level of the test
     * it has to wait new observations to increase the number of residuals after the change to perform the test.
     * In the latter case the observations are discarded anyway by the beholder, because they are not used
     * for the 1st level test, but they are recorded in the trace thanks to agorà.
     * Thus hopefully after a possible wait period the beholder finds enough observations after-the-change
     * in the trace.
     */
    COMPUTING,

    /**
     * @details
     * In this state the ApplicationHandler is disabled, meaning that any action involving the handler
     * is discarded. It is only allowed to receive the kia message from clients in order to keep
     * the list of client up-to-date.
     * The handlers enter this state when agorà goes offline (kia message from agora).
     * The handlers exits from this state only when agorà comes back online.
     * In order to manage the save/restore of the situation of the handlers across agora's lifecycle
     * we use a copy of the status of the handler "previous_status". When agorà goes offline the status
     * is saved in the "previous_status" variable.
     * When agorà comes back online the status is restored. This is the only use of the "previous_status" variable.
     */
    DISABLED,

    /**
     * @details
     * In this state the ApplicationHandler is waiting for a new broadcast-model message for its application,
     * after it has taken the decision to trigger a retraining.
     * Thus the handler discard any observations (possibly belonging to clients with still the old model as knowledge).
     * The handler is re-enabled only by the correct message broadcasted by agorà informing about the new model.
     * The only difference between this state and the "RETRAINING" one is that in this state the handler knows
     * that the retraining message has already been sent to agorà.
     */
    TRAINING,

    /**
     * @details
     * In this state the ApplicationHandler has taken the decision to trigger a retraining, but it
     * cannot issue it right away because agorà is offline. This state would be saved as a previous_status,
     * and when agorà would come back online the status would be restored and the retraining message would be
     * issued. Other than that the handler behaves as it it were in the TRAINING status, meaning that
     * it discards any incoming message. Once agorà comes back online and the handler are re-enabled
     * the retraining message is sent to agorà and the handler switches to the TRAINING status.
     */
    RETRAINING
  };

  class RemoteApplicationHandler
  {


    private:

      // to protect the data structure
      std::mutex mutex;

      /**
       * @details
       * Suffix counter for the exported files, so that at every retraining or reset of the ici-test
       * a new folder (equal to this counter) is created and new files are created from scratch.
       * The old files are left intact so that the user is able to inspect the files
       * at a later date and check the behaviour and the situation of the application at every stage.
       * THe user can manually plot the ici curves, manually perform the hypothesis test
       * and check the consistency of the decisions taken by the beholder.
       * This counter is initialized at 1 and is increased at the end of every hypothesis test,
       * whether the change is rejected (ici test reset) or the retraining is issued (change confirmed).
       */
      int suffix_plot;

      /**
       * @details
       * Counter of the total observations used in the current ici test.
       * It gets reset at the end of every hypothesis test (i.e. every retraining or reset of the ici test).
       * Following a reset of the ici test (change rejected) it gets re-initialized at the number of observations used
       * for training, because the training phase of the ici test is kept.
       * Following a retraining is gets zeroed.
       * This counter does not take into account observations coming from blacklisted clients
       * or observations arrived while the handler was busy (status!=READY).
       * It does not take into account the first observation received from a new client too.
       */
      int current_test_observations_counter;

      /**
       * @details
       * Counter of the total number of observations used by this handler across all the ici tests.
       * It is never reset.
       * This counter does not take into account observations coming from blacklisted clients
       * or observations arrived while the handler was busy (status!=READY).
       * It does not take into account the first observation received from a new client too.
       */
      int observations_counter;

      /**
       * @details
       * Counter of the number of times the re-training has been issued by this handler.
       * It measures the total number of times the ici test was right in detecting a change.
       */

      int retraining_counter;

      /**
       * @details
       * Counter of the number of times the ici test has been reset in this handler.
       * It measures the number of times the ici test has been rejected by the 2nd leve test.
       */
      int ici_reset_counter;

      // application-specific root workspace path
      std::string application_workspace;

      /**
       * @details
       * This hashmap maps each metric name (key) to a struct containing the files used to export
       * the data used by the ici test.
       * The struct's first element "observations" is the file containing all the residuals collected
       * and used in the ICI change detection test,
       * the second element "ici" contains the ICI data for the mean and for the variance of every window of the CDT.
       * These two files together allow the user to plot the ICI curves with the provided gnuplot script.
       * The creation of these files is controlled by the beholder cli option "--output_files".
       */
      std::unordered_map<std::string, output_files> output_files_map;

      // to handle the progress of the elaboration
      ApplicationStatus status;

      // to save the current application status when an handler is paused following agorà's kia message
      // The status will be then set to DISABLED.
      ApplicationStatus previous_status;

      // these is the data structure containing information about the application, like the metrics...
      application_description_t description;

      // Prefix to log strings  for this handler containing the application name
      std::string log_prefix;

      /**
       * @details
       * Clients blacklist, implemented as an unordered set since we do not need any sorting.
       * A client joins this set only when a change is detected, but the change is overall rejected
       * because the number of clients behaving badly is lower than the user-set threshold.
       * The bad clients then join this set and the observations coming from them are not considered
       * anymore for the ici test. This set is cleared when a new change is detected.
       * All the clients (including those which were blacklisted) will perform the 2nd level test again,
       * and it starts all over again with an empty blacklist.
       */
      std::unordered_set<std::string> clients_blacklist;

      /**
       * @details
       * Hashmap storing the list of clients encountered by this handler.
       * The client-id is the hash key, while the value is a struct representing a timestamp.
       * The first field of the timestamp is the "seconds", while the second field represents the "nanoseconds".
       * The timestamp contains the time of the first observation coming from the corresponding client.
       * This will be used in the 2nd level test to filter the trace and gather rows
       * with timestamp greater than this one.
       */
      std::unordered_map<std::string, timestamp_fields> clients_list;


      /**
       * @details
       * Set containing the metrics observed by the beholder
       * NB: these are not necessarily all the available metrics, or all the ENABLED metrics,
       * but are instead the metrics that the user explicitely set to be monitored
       * by the beholder in the XML config file.
       * This is to say that it could be the same as all the currently available metrics,
       * but it also may be different, according to the user settings.
       */
      std::set<std::string> reference_metric_names;

      /**
       * @details
       * Hashmap to store the residuals from the observations received,
       * i.e. the difference between the predicted value by the model and the actual value.
       * This structure maps every metric name (key) to a vector of structs.
       * The vector of structs is the buffer of residuals for the metric of interest.
       * The buffer will be as big as the beholder (user-set) option "window_size" instructs.
       * The struct's first element is the residual value itself.
       * The struct's second element is a another struct containing the timestamp of the corresponding residual,
       * it has two members: two strings containing respectively the seconds since epoch and nanoseconds.
       * The timestamp of the first and last element in the current window allows us to pinpoint
       * the change time range later on in Cassandra's trace.
       * We could have buffers (for different metrics) with different level of filling,
       * according to the availability of metrics in the incoming observations,
       * this is the reason why the timestamps of the first and last element of the window are not unique
       * across the whole application handler but are instead metric-specific.
       */
      std::unordered_map<std::string, std::vector<residual_struct>> residuals_map;

      // ICI CDT hashmap: it maps every metric name (key) to its struct of data for the ICI test.
      std::unordered_map<std::string, Data_ici_test> ici_cdt_map;

      // Struct containing the timestamps (in Ctime's format) of the first and last element of
      // the change window selected by the ici test.
      window_timestamps change_window_timestamps;

      // Name of the metric whose ici test has first triggered the detection of the change.
      std::string change_metric_name;

      // these are the functions used to communicate using MQTT topics
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
      void second_level_test( std::unordered_map<std::string, timestamp_fields>& clients_list_snapshot );
      void parse_and_insert_observations_for_client_from_trace(std::unordered_map<std::string, residuals_from_trace>& client_residuals_map, const observation_t j,
          const std::set<std::string>& metric_to_be_analyzed);

      /**
       * @details
       * Function which actually issues the retraining.
       * It only has to send the correct message to agorà.
       * There are two versions of the retraining command.
       * The default one is the simplest and instructs agorà to truncate the trace table.
       * The other version (enabled by the beholder user-set cli option "--no_trace_drop")
       * append to the command the timestamp of the last element of the change window.
       * In this way agorà will delete just the part of the trace before that timestamp.
       */
      inline void retraining ( void )
      {
        // need to trigger RE-training
        // this automatically deals with the deletion of the model and of the trace and with the reset of the doe
        // in this version the trace is deleted just from the top element in the table up to the last element of the training window
        if (Parameters_beholder::no_trace_drop)
        {
          agora::pedantic(log_prefix, "Deleting the model, restoring the DOE, deleting just the rows of the trace which are before the detected change window.");
          send_agora_command("retraining " + change_window_timestamps.back.seconds + "," + change_window_timestamps.back.nanoseconds);
        }
        else
        {
          agora::pedantic(log_prefix, "Deleting the model, restoring the DOE, deleting the whole trace.");
          // delete the whole trace
          send_agora_command("retraining");
        }

        change_window_timestamps = {};
        retraining_counter++;
        status = ApplicationStatus::TRAINING;
      }

    public:

      /**
       * @details
       * Re-activate the handler following the reception of a new model.
       * This method is used only after a re-training was issued by this same handler and
       * the handler was set to TRAINING.
       */
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

      /**
       * @details
       * Function which pauses the handler after agorà's kia message reception setting the status to DISABLED.
       * Here the "previous_status" gets used to backup the original status before the disabling of the handler.
       */
      inline void pause_handler ( void )
      {
        // lock the mutex to ensure a consistent global state
        std::unique_lock<std::mutex> guard(mutex);

        // only if the current status is not already DISABLED, otherwise I lose the original previous_status
        if (status != ApplicationStatus::DISABLED)
        {
          agora::info(log_prefix, "Handler put-on-hold after agorà's' kia. Waiting for agorà's resurrection...");
          previous_status = status;
          status = ApplicationStatus::DISABLED;
        }
      }

      /**
       * @details
       * Function which un-pauses the handler after agorà's welcome message reception (resurrection).
       * The status gets restored to the one before the disabling.
       * Here the "previous_status" gets used to restore the original status.
       */
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
            retraining();
          }
        }
      }


      /**
       * @details
       * Function which removes the client received as parameter from the list of active clients
       * encountered by the beholder.
       * It is called after a client's kia message reception.
       */
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

      /**
       * @details
       * Default ApplicationHandler constructor.
       * It sets the status of the handler to READY and it initializes the counters and the workspace folder.
       * It also writes a file containing the configuration with the current user-set parameters
       * used by the beholder to allow the user to manually plot the ICI curves (with the provided gnuplot script)
       * with the same configuration used inside the framework.
       */
      RemoteApplicationHandler( const std::string& application_name );

      /**
       * @details
       * This is the core method, the orchestrator of all the logic and the decisions
       * for the target application from the viewpoint of the beholder's tasks.
       * This method is responsible for receiving the observations coming from the client applications
       * and for taking care of their parsing and their insertion in the memory buffers.
       * If the buffers are filled-in then the first level (ici) test is carried out.
       * According to the outcome of the first level test the it could perform
       * the 2nd level test (hypothesis test).
       * Depending on the outcome of the 2nd level test it can trigger the retraining,
       * which is the most important (and impactful) decision the beholder can possibly take.
       * The retraining itself is the real purpose of the beholder, and has to be triggered
       * only under some circumstances.
       */
      void new_observation( const std::string& values );

  };
}

#endif // MARGOT_BEHOLDER_APPLICATION_HANDLER_HDR
