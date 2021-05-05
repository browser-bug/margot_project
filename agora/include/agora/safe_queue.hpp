/* Agora library
 * Copyright (C) 2021 Bernardo Menicagli
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

#ifndef MARGOT_AGORA_SAFE_QUEUE_HPP
#define MARGOT_AGORA_SAFE_QUEUE_HPP

#include <condition_variable>
#include <deque>
#include <mutex>

namespace agora {

template <class T>
class Queue {
public:
    Queue() : signal_terminate(false) {}

    // this queue is designed to be used safely, cannot be moved though
    Queue(const Queue &) = delete;
    Queue &operator=(const Queue &) = delete;

    virtual ~Queue() { signal_terminate = true; }

    inline std::size_t size() const {
        std::unique_lock<std::mutex> lock(job_queue_mutex);
        return job_queue.size();
    }

    inline bool empty() const {
        std::unique_lock<std::mutex> lock(job_queue_mutex);
        return job_queue.empty();
    }

    inline void send_terminate_signal() {
        std::unique_lock<std::mutex> lock(job_queue_mutex);
        signal_terminate = true;
        job_queue_cv.notify_all();
    }

    inline void clear_terminate_signal() {
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
    bool dequeue(T &output_job) {
        // get the lock on the operation
        std::unique_lock<std::mutex> lock(job_queue_mutex);

        // look until there is some work to do
        // please, note that spurious wake up happens!
        while (!signal_terminate && job_queue.empty()) {
            job_queue_cv.wait(lock);  // release lock -> wait for a wake_up -> reaquire lock
        }

        // check if the termination condition is met
        if (signal_terminate) {
            return false;
        }

        // get the first available job
        output_job = job_queue.back();
        job_queue.pop_back();

        // return with a meaningful job
        return true;
    }

    void enqueue(const T &input_job) {
        // get the lock on the operation
        std::unique_lock<std::mutex> lock(job_queue_mutex);

        // actually enqueue the new element
        job_queue.emplace_front(input_job);

        // wake up if waiting on an empty queue
        job_queue_cv.notify_one();
    }

private:
    std::deque<T> job_queue;

    bool signal_terminate;

    // synchronization variables
    mutable std::mutex job_queue_mutex;
    std::condition_variable job_queue_cv;
};

}  // namespace agora

#endif  // MARGOT_AGORA_SAFE_QUEUE_HPP
