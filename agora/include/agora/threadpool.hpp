#ifndef MARGOT_AGORA_THREADPOOL_HPP
#define MARGOT_AGORA_THREADPOOL_HPP

#include <vector>

#include "agora/worker.hpp"

namespace agora {

class ThreadPool {

public:
  ThreadPool(size_t number_of_workers = std::thread::hardware_concurrency())
  {
    threads.reserve(number_of_workers);

    for (size_t i = 0; i < number_of_workers; ++i)
    {
      threads.push_back(std::make_unique<Worker>(std::to_string(i)));
    }
  }

  // std::thread objects are not copiable so we don't want a thread pool to be copied neither
  ThreadPool(const ThreadPool &) = delete;
  ThreadPool &operator=(const ThreadPool &) = delete;

  // let's make sure that on destruction we're waiting all workers
  ~ThreadPool() { wait_workers(); }

  void start_workers()
  {
    // spawn all the threads
    for (auto &worker : threads)
    {
      worker->start();
    }
  }

  void wait_workers()
  {
    for (auto &worker : threads)
    {
      if(worker->is_running()){
        worker->wait();
      }
    }
  }

  void stop_workers()
  {
    for (auto &worker : threads)
    {
      worker->stop();
    }
  }

private:
  std::vector<std::unique_ptr<Worker>> threads;
};
} // namespace agora

#endif // MARGOT_AGORA_THREADPOOL_HPP
