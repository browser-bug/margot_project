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

#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <vector>

#include "agora/worker.hpp"

namespace agora {

/**
 * @brief A thread pool of Agora Worker threads.
 *
 * @details
 * Agora is based on a thread pool design that achieves concurrency of execution inside the system. It maintains multiple threads waiting
 * for tasks to be allocated. In this way the system increases performance and avoids latency in execution.
 */
class ThreadPool {
public:
    /**
     * @brief Create a new thread pool instance.
     *
     * @param [in] number_of_workers The number of threads to spawn (defaults to the system hardware max concurrency).
     */
    ThreadPool(size_t number_of_workers = std::thread::hardware_concurrency()) {
        threads.reserve(number_of_workers);

        for (size_t i = 0; i < number_of_workers; ++i) {
            threads.push_back(std::make_unique<Worker>(std::to_string(i)));
        }
    }

    // std::thread objects are not copyable so we don't want a thread pool to be copied neither
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;

    /**
     * @brief Destruct the instance waiting all the spawned threads.
     */
    ~ThreadPool() { wait_workers(); }

    /**
     * @brief Start the task assigned on all available threads.
     */
    void start_workers() {
        // spawn all the threads
        for (auto &worker : threads) {
            worker->start();
        }
    }

    /**
     * @brief Wait the assigned task on all running threads.
     */
    void wait_workers() {
        for (auto &worker : threads) {
            if (worker->is_running()) {
                worker->wait();
            }
        }
    }

    /**
     * @brief Interrupt the assigned task on all running threads.
     */
    void stop_workers() {
        for (auto &worker : threads) {
            worker->stop();
        }
    }

private:
    /**
     * @brief A list of available/running threads.
     */
    std::vector<std::unique_ptr<Worker>> threads;
};
}  // namespace agora

#endif // THREADPOOL_HPP
