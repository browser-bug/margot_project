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

#ifndef MARGOT_AGORA_THREADPOOL_HPP
#define MARGOT_AGORA_THREADPOOL_HPP

#include <vector>

#include "agora/worker.hpp"

namespace agora {

class ThreadPool {
public:
    ThreadPool(size_t number_of_workers = std::thread::hardware_concurrency()) {
        threads.reserve(number_of_workers);

        for (size_t i = 0; i < number_of_workers; ++i) {
            threads.push_back(std::make_unique<Worker>(std::to_string(i)));
        }
    }

    // std::thread objects are not copiable so we don't want a thread pool to be copied neither
    ThreadPool(const ThreadPool &) = delete;
    ThreadPool &operator=(const ThreadPool &) = delete;

    // let's make sure that on destruction we're waiting all workers
    ~ThreadPool() { wait_workers(); }

    void start_workers() {
        // spawn all the threads
        for (auto &worker : threads) {
            worker->start();
        }
    }

    void wait_workers() {
        for (auto &worker : threads) {
            if (worker->is_running()) {
                worker->wait();
            }
        }
    }

    void stop_workers() {
        for (auto &worker : threads) {
            worker->stop();
        }
    }

private:
    std::vector<std::unique_ptr<Worker>> threads;
};
}  // namespace agora

#endif  // MARGOT_AGORA_THREADPOOL_HPP
