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

#ifndef SAFE_QUEUE_HPP
#define SAFE_QUEUE_HPP

#include <condition_variable>
#include <deque>
#include <mutex>

namespace agora {

/**
 * @brief A synchronized queue.
 *
 * @tparam T The type of data stored in the queue.
 *
 * @details
 * This class is being used primary for message storing but can be extended without effort to any kind of data. All its internal methods are
 * thread-safe to ensure synchronization.
 */
template <class T>
class Queue {
public:
    Queue() : signal_terminate(false) {}

    // this class should not be copied or moved around
    Queue(const Queue &) = delete;
    Queue &operator=(const Queue &) = delete;

    virtual ~Queue() { signal_terminate = true; }

    /**
     * @brief Get the current queue size.
     *
     * @returns The number of elements currently stored.
     */
    std::size_t size() const {
        std::unique_lock<std::mutex> lock(job_queue_mutex);
        return job_queue.size();
    }

    /**
     * @brief Check if the queue is empty.
     *
     * @returns True if no elements are stored, False otherwise.
     */
    bool empty() const {
        std::unique_lock<std::mutex> lock(job_queue_mutex);
        return job_queue.empty();
    }

    /**
     * @brief Notify all waiting threads that the queue is inactive and hence to terminate.
     */
    void send_terminate_signal() {
        std::unique_lock<std::mutex> lock(job_queue_mutex);
        signal_terminate = true;
        job_queue_cv.notify_all();
    }

    /**
     * @brief Set the queue to active state.
     */
    void clear_terminate_signal() {
        std::unique_lock<std::mutex> lock(job_queue_mutex);
        signal_terminate = false;
    }

    /**
     * @brief Wait until there is some work available or if the thread should terminate.
     *
     * @param [out] output_job The job that needs to be assigned to the thread.
     *
     * @returns True if there is a job available, False otherwise.
     */
    bool dequeue(T &output_job) {
        // get the lock on the operation
        std::unique_lock<std::mutex> lock(job_queue_mutex);

        // look until there is some work to do
        // please, note that spurious wake up happens!
        while (!signal_terminate && job_queue.empty()) {
            job_queue_cv.wait(lock);  // release lock -> wait for a wake_up -> require lock
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

    /**
     * @brief Put a new job in the queue waiting if it is being used.
     *
     * @param [in] input_job The job that needs to be put in the queue.
     */
    void enqueue(const T &input_job) {
        // get the lock on the operation
        std::unique_lock<std::mutex> lock(job_queue_mutex);

        // actually enqueue the new element
        job_queue.emplace_front(input_job);

        // wake up if waiting on an empty queue
        job_queue_cv.notify_one();
    }

private:
    /**
     * @brief The queue storing the available jobs.
     */
    std::deque<T> job_queue;

    /**
     * @brief Store True if the queue is active, False otherwise.
     */
    bool signal_terminate;

    // synchronization variables
    mutable std::mutex job_queue_mutex;
    std::condition_variable job_queue_cv;
};

}  // namespace agora

#endif // SAFE_QUEUE_HPP
