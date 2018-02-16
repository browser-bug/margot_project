/* agora/threadpool.hpp
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

#ifndef MARGOT_AGORA_THREADPOOL_HDR
#define MARGOT_AGORA_THREADPOOL_HDR

#include <thread>
#include <vector>
#include <cstdint>

#include "remote_handler.hpp"
#include "virtual_channel.hpp"
#include "logger.hpp"


namespace margot
{

  template< class T >
  class ThreadPool
  {

    private:

      VirtualChannel channel;
      std::vector<std::thread> pool;
      T worker_functor;

    public:

      template< class... Ts >
      ThreadPool( VirtualChannel target_channel, uint16_t number_of_workers, const Ts... arguments)
        : channel(target_channel), worker_functor(target_channel, arguments...)
      {

        // build the lambda which defines the actual thread function
        auto worker_wrapper = [this] (void)
        {
          // notify that we are a new thread
          margot::info("Thread ", std::this_thread::get_id(), " on duty");

          // assuming that there is plenty of work for everybody
          while (true)
          {

            // declaring the new message
            message_t new_incoming_message;

            if (!channel.recv_message(new_incoming_message))
            {
              margot::info("Thread ", std::this_thread::get_id(), " on retirement");
              return; // there is no more work available
            }

            // otherwise process the incoming message
            worker_functor(new_incoming_message);
          }
        };

        // spawn all the threads
        pool.reserve(number_of_workers);

        for ( uint16_t i = 0; i < number_of_workers; ++i)
        {
          pool.emplace_back(std::thread(worker_wrapper));
        }
      }

      ~ThreadPool( void );

      void force_disconnect( void );

      void wait_workers( void );

  };


  template< class T >
  ThreadPool<T>::~ThreadPool( void )
  {
    force_disconnect();
    wait_workers();
  }

  template< class T >
  void ThreadPool<T>::force_disconnect( void )
  {
    // to wake up all the threads it is enough to close the communication
    // channel. To create a new one, we must rehestabilish a connection
    channel.destroy_channel();
  }

  template< class T >
  void ThreadPool<T>::wait_workers( void )
  {
    for ( auto& worker : pool)
    {
      if (worker.joinable())
      {
        worker.join();
      }
    }
  }

}

#endif // MARGOT_AGORA_THREADPOOL_HDR
