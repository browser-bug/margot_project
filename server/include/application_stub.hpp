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
#include <random>
#include <iostream>

#include "virtual_channel.hpp"
#include "paho_remote_implementation.hpp"
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
    VirtualChannel channel;


    // this is the function executed by the main thread
    void local_application_handler( void )
    {
      margot::info("ToyApp thread ", std::this_thread::get_id(), " on duty");

      // remember, this is a thread, it shoul terminates only when the
      // client disconnect
      while (true) {

        // declaring the new message
        message_t new_incoming_message;

        if (!channel.recv_message(new_incoming_message))
        {
          margot::info("ToyApp thread ", std::this_thread::get_id(), " on retirement ");
          return; // there is no more work available
        }

        // otherwise process the incoming message
        std::cout << "********* MARGOT LOCAL HANDLER WORK ***********" << knob1 << std::endl;
      }
    }

  public:

    MargotMimicking( void ):knob1(1),knob2(2),knob3(3){}

    ~MargotMimicking( void )
    {
      channel.destroy_channel();
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
    }

    // this function should be parametric and hidden
    void start_support_thread( void )
    {
      // initialize communication channel with the server
      channel.create_channel<PahoClient>("127.0.0.1:1883", 0);

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
    MargotMimicking autotuner;

    // to generate random numbers
    std::random_device random_device;
    std::default_random_engine generator;
    std::uniform_real_distribution<float> feature1_distribution;
    std::uniform_real_distribution<float> feature2_distribution;
    std::uniform_int_distribution<int> time_error_distribution;

  public:

    Application( void )
      :autotuner()
      ,random_device(),generator(random_device()),feature1_distribution(1.0f,6.0f)
      ,feature2_distribution(10.0f,20.0f),time_error_distribution(1,10)
    {}


    inline int do_job( void ) const
    {
      return knob1*100 + knob2*10 + knob3 * (feature1*feature2);
    }



    inline void operator()( const std::chrono::seconds duration = std::chrono::seconds(200) )
    {
      // this is the equivalent of the main function of the application

      // in the init we must initialize the autotuner, here we spawn a thread
      autotuner.start_support_thread();

      // taking the time of now
      const auto start_time = std::chrono::steady_clock::now();
      const auto stop_time = start_time + duration;

      // loop to simulate an application running
      while( std::chrono::steady_clock::now() < stop_time )
      {
        // take the configuration from the autotuner
        autotuner.update(knob1, knob2, knob3);

        // "read" the input feature from the input file
        feature1 = feature1_distribution(generator);
        feature2 = feature2_distribution(generator);

        // "run" the application
        const auto execution_time = do_job();
        std::this_thread::sleep_for(std::chrono::milliseconds(execution_time));
        info("APPLICATION: k1=", knob1, " k2=", knob2, " k3=", knob3,
             " f1=", feature1, " f2=", feature2, " time=", execution_time);

      }
    }

  };

}

#endif // MARGOT_AGORA_APPLICATION_STUB_HDR
