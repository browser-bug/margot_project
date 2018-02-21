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


namespace margot
{

  class ThreadPool
  {

    private:

      std::vector<std::thread> pool;

    public:

      template< class Function, class... Arguments >
      ThreadPool( uint16_t number_of_workers, Function&& f,  Arguments&& ... args)
      {
        // spawn all the threads
        pool.reserve(number_of_workers);

        for ( uint16_t i = 0; i < number_of_workers; ++i)
        {
          pool.emplace_back(std::thread(f, args...));
        }
      }

      ~ThreadPool( void );

      void wait_workers( void );

  };


  ThreadPool::~ThreadPool( void )
  {
    wait_workers();
  }

  void ThreadPool::wait_workers( void )
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
