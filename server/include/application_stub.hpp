/* agora/fs_handler.hpp
 * Copyright (C) 2018 Davide Gadioli
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

#ifndef MARGOT_AGORA_APPLICATION_STUB_HDR
#define MARGOT_AGORA_APPLICATION_STUB_HDR

#include <string>
#include <thread>
#include <mutex>
#include <memory>
#include <chrono>
#include <ctime>
#include <random>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>

#include "virtual_channel.hpp"
#include "logger.hpp"

namespace margot
{


  class MargotMimicking
  {
      // those are the parameters to change
      std::mutex parameter_change;
      int knob1;
      int knob2;
      int knob3;

      // those are the last value of the metrics
      int execution_time;

      // those are the value of the input features
      float feature1;
      float feature2;

      // this is a reference to the support thread
      std::thread local_handler;
      VirtualChannel remote;
      std::string application_name;


      // this is the function executed by the main thread
      void local_application_handler( void )
      {
        margot::info("mARGOt support thread on duty");

        // initialize communication channel with the server
        remote.create<PahoClient>(application_name, "127.0.0.1:1883", 0);

        // get my own id
        const std::string my_client_id = remote.get_my_client_id();

        // register to the application-specific topic (before send the welcome message)
        remote.subscribe("margot/" + application_name + "/" + my_client_id + "/#");

        // register to the application to receive the model
        remote.subscribe("margot/" + application_name + "/model");

        // announce to the world that i exists
        remote.send_message({{"margot/" + application_name + "/welcome"}, my_client_id});

        // remember, this is a thread, it should terminate only when the
        // client disconnect, so keep running until the application is up
        while (true)
        {
          // declaring the new message
          message_t new_incoming_message;

          if (!remote.recv_message(new_incoming_message))
          {
            margot::info("mARGOt support thread on retirement");
            return; // there is no more work available
          }

          // get the "topic" of the message
          const auto start_type_pos = new_incoming_message.topic.find_last_of('/');
          const std::string message_topic = new_incoming_message.topic.substr(start_type_pos);

          // handle the info message
          if (message_topic.compare("/info") == 0)
          {
            const std::vector< std::string > descriptions = {
              "knob      primus int 1 2 3@",
              "knob      secundus int 4 5 6@",
              "knob      terzius int 7 8 9@",
              "feature   destrezza float 1 3.5 6@",
              "feature   costituzione float 10 15 20@",
              "metric    exec_time int rgam@",
              "doe       full_factorial@",
              "num_obser 1"
            };
            std::ostringstream os;
            std::for_each(descriptions.begin(), descriptions.end(), [&os] ( const std::string& configuration )
            { os << configuration; });
            remote.send_message({{"margot/" + application_name + "/info"},os.str()});
          }

          // handle the configurations incoming from the server
          if (message_topic.compare("/explore") == 0)
          {
            std::stringstream stream(new_incoming_message.payload);
            stream >> knob1;
            stream >> knob2;
            stream >> knob3;
          }

        }
      }

    public:

      MargotMimicking( void ): knob1(1), knob2(2), knob3(3) {}

      ~MargotMimicking( void )
      {
        remote.destroy_channel();

        if (local_handler.joinable())
        {
          local_handler.join();
        }
      }


      inline void update( int& k1, int& k2, int& k3 )
      {
        std::lock_guard<std::mutex> lock(parameter_change);
        k1 = knob1;
        k2 = knob2;
        k3 = knob3;
      }

      inline void stop_monitor( float feature1, float feature2, int execution_time )
      {
        this->feature1 = feature1;
        this->feature2 = feature2;
        this->execution_time = execution_time;

        // take the time of now
        auto now = std::chrono::system_clock::now();

        // get the number of seconds and nanoseconds since epoch
        const auto sec_since_now = std::chrono::duration_cast< std::chrono::seconds >(now.time_since_epoch());
        const auto almost_epoch = now - sec_since_now;
        const auto ns_since_sec = std::chrono::duration_cast< std::chrono::nanoseconds >(almost_epoch.time_since_epoch());

        // notify the server about the performance
        std::string&& payload = std::to_string(sec_since_now.count()) + ","
                                + std::to_string(ns_since_sec.count()) + " "
                                + remote.get_my_client_id() + " "
                                + std::to_string(knob1) + "," + std::to_string(knob2) + "," + std::to_string(knob3) + " "
                                + std::to_string(feature1) + "," + std::to_string(feature2) + " "
                                + std::to_string(execution_time);
        remote.send_message({{"margot/" + application_name + "/observation"},payload});
      }

      // this function should be parametric and hidden
      void start_support_thread( const std::string& application_name )
      {
        // get the application name
        this->application_name = application_name;

        // start the thread
        local_handler = std::thread(&MargotMimicking::local_application_handler, this);
      }

  };




  class Application
  {
    private:

      int knob1;
      int knob2;
      int knob3;

      float feature1;
      float feature2;

      // this objects emulates the margot client
      std::shared_ptr<MargotMimicking> autotuner;

      // to generate random numbers
      std::default_random_engine generator;
      std::uniform_real_distribution<float> feature1_distribution;
      std::uniform_real_distribution<float> feature2_distribution;
      std::uniform_int_distribution<int> time_error_distribution;

    public:

      Application( void )
        : generator(std::random_device()()), feature1_distribution(1.0f, 6.0f)
        , feature2_distribution(10.0f, 20.0f), time_error_distribution(1, 10)
      {}


      inline int do_job( void ) const
      {
        return knob1 * 100 + knob2 * 10 + knob3 * (feature1 * feature2);
      }



      inline void operator()( const std::chrono::seconds duration = std::chrono::seconds(200) )
      {
        // this is the equivalent of the main function of the application

        // in the init we must initialize the autotuner, here we spawn a thread
        autotuner.reset( new MargotMimicking() );
        autotuner->start_support_thread("swaptions/v1_3/elaboration");

        // taking the time of now
        const auto start_time = std::chrono::steady_clock::now();
        const auto stop_time = start_time + duration;

        // loop to simulate an application running
        while ( std::chrono::steady_clock::now() < stop_time )
        {
          // take the configuration from the autotuner
          autotuner->update(knob1, knob2, knob3);

          // "read" the input feature from the input file
          feature1 = feature1_distribution(generator);
          feature2 = feature2_distribution(generator);

          // "run" the application
          const auto execution_time = do_job();
          std::this_thread::sleep_for(std::chrono::milliseconds(execution_time));
          info("APPLICATION: k1=", knob1, " k2=", knob2, " k3=", knob3,
               " f1=", feature1, " f2=", feature2, " time=", execution_time);

          // stop the measurement and log to the application
          autotuner->stop_monitor(feature1, feature2, execution_time);
        }
      }

  };

}

#endif // MARGOT_AGORA_APPLICATION_STUB_HDR
