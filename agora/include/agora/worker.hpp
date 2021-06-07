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

#ifndef WORKER_HPP
#define WORKER_HPP

#include <condition_variable>
#include <thread>
#include <vector>

// thread utility headers
#include <csignal>
#include <sys/syscall.h>
#include <unistd.h>

#include "agora/logger.hpp"
#include "agora/model_message.hpp"
#include "agora/remote_handler.hpp"

namespace agora {

/**
 * @brief An Agora worker thread.
 *
 * @details
 * This class provides a support to initialize and track Agora support threads. A worker is a thread which is started initially and run till
 * the Agora executable is stopped. This class provides support to name worker threads and start/terminate them. Every worker thread is
 * assigned with a task which correspond to the handling of incoming messages from clients exploiting the usage of the
 * RemoteApplicationHandler.
 */
class Worker {
public:
    /**
     * @brief Construct a new worker instance.
     *
     * @param [in] name The name of the thread.
     */
    Worker(const std::string &name);
    /**
     * @brief Destruct the worker instance and wait until its task is finished/terminated.
     */
    ~Worker();

    // std::thread objects are not copyable so we don't want a worker object to be copied neither
    Worker(const Worker &) = delete;
    Worker &operator=(const Worker &) = delete;

    /**
     * @brief Start the worker task.
     *
     * @details
     * The code defined inside Worker::task() will be executed after calling this method.
     */
    void start();
    /**
     * @brief Wait the worker task.
     *
     * @returns True if the worker should continue, False otherwise.
     */
    bool wait();
    /**
     * @brief Stop the worker task.
     */
    void stop();
    /**
     * @brief Check if the worker thread is still running.
     *
     * @returns True if it's running, False otherwise.
     */
    bool is_running() { return !finished; }

    /**
     * @brief Get the name of the worker.
     *
     * @returns The name assigned to the worker.
     */
    const std::string get_name() const { return name; }
    /**
     * @brief Get the thread ID of the process thread.
     *
     * @returns The thread identifier assigned to the process.
     */
    const pid_t get_tid() const { return worker_tid; }

private:
    /**
     * @brief The name of the worker thread.
     */
    std::string name;

    /**
     * @brief Set True if the thread is terminated, False otherwise.
     */
    bool finished;
    /**
     * @brief Mutex controlling the worker execution.
     */
    std::mutex worker_mtx;
    /**
     * @brief Conditional variable used to signal the worker thread.
     */
    std::condition_variable worker_cv;

    /**
     * @brief The worker thread.
     */
    std::thread worker_thd;
    /**
     * @brief The worker thread identifier.
     */
    pid_t worker_tid;
    /**
     * @brief Set a new thread ID for the process thread.
     *
     * @param [in] tid The new thread identifier to assign.
     */
    void set_tid(const pid_t &tid) { worker_tid = tid; }

    /**
     * @brief The worker thread main code.
     *
     * @details
     * This task is common to all spawned threads and loops over waiting for the reception of a new message from the outside until a
     * terminating signal is notified. Inside this method the Worker::handle_incoming_message() function or the
     * Worker::handle_system_message() are called depending on the message type.
     */
    void task();
    /**
     * @brief Manage a system message.
     *
     * @param [in] client_id The client identifier sending the message.
     * @param [in] command_message A string containing the command to execute.
     */
    void handle_system_message(const std::string &client_id, const std::string &command_message);
    /**
     * @brief Manage a general message.
     *
     * @param [in] new_message The data structure containing the topic and payload of the message.
     */
    void handle_incoming_message(const message_model &new_message);

    /**
     * @brief Extract the application ID from the topic of a message.
     *
     * @param [in] topic A string containing the message topic.
     * @param [in] app_id_separator A character separating a series of tokens inside the message topic.
     *
     * @returns The application ID extracted.
     */
    const application_id get_application_id(const std::string &topic, std::string app_id_separator = "^");

    /**
     * @brief A pointer to the global Logger.
     */
    std::shared_ptr<Logger> logger;
    /**
     * @brief A pointer to the remote message handler.
     */
    std::shared_ptr<RemoteHandler> remote;
};

}  // namespace agora

#endif // WORKER_HPP
