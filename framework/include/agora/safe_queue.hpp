/* agora/safe_queue.hpp
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

#ifndef MARGOT_AGORA_SAFE_QUEUE_HDR
#define MARGOT_AGORA_SAFE_QUEUE_HDR

#include <deque>
#include <mutex>
#include <condition_variable>

namespace agora
{

  template< class T >
  class Queue
  {
    private:

      bool signal_terminate;
      mutable std::mutex job_queue_mutex;
      std::condition_variable work_available;
      std::deque<T> job_queue;

    public:

      Queue(void): signal_terminate(false) {}

      // this queue is designed to be used safely, cannot be moved though
      Queue( const Queue& ) = delete;
      Queue( Queue&& ) = delete;

      inline std::size_t size( void ) const
      {
        std::unique_lock<std::mutex> lock(job_queue_mutex);
        return job_queue.size();
      }

      inline bool empty( void ) const
      {
        std::unique_lock<std::mutex> lock(job_queue_mutex);
        return job_queue.empty();
      }

      inline void send_terminate_signal( void )
      {
        std::unique_lock<std::mutex> lock(job_queue_mutex);
        signal_terminate = true;
        work_available.notify_all();
      }

      inline void clear_terminate_signal( void )
      {
        std::unique_lock<std::mutex> lock(job_queue_mutex);
        signal_terminate = false;
      }

      /**
       * @brief wait until there some work available or the thread should terminate
       *
       * @param [out] output_job The job assigned to the thread
       *
       * @return true if the job is really a work to do, otherwise false
       */
      bool dequeue( T& output_job )
      {
        // get the lock on the operation
        std::unique_lock<std::mutex> lock(job_queue_mutex);

        // look until there is some work to do
        // please, note that spurious wake up happens!
        while (!signal_terminate && job_queue.empty())
        {
          work_available.wait(lock);  // release lock -> wait for a wake_up -> reaquire lock
        }

        // check if the termination condition is met
        if (signal_terminate)
        {
          return false;
        }

        // get the first available job
        output_job = job_queue.back();
        job_queue.pop_back();

        // return with a meaningful job
        return true;
      }

      void enqueue( T input_job )
      {
        // get the lock on the operation
        std::unique_lock<std::mutex> lock(job_queue_mutex);

        // actually enqueue the new element
        job_queue.emplace_front(input_job);

        // wake up an element
        work_available.notify_one();
      }

  };

}


#endif // MARGOT_AGORA_SAFE_QUEUE_HDR
