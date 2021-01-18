#ifndef MARGOT_AGORA_WORKER_HPP
#define MARGOT_AGORA_WORKER_HPP

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

class Worker {
public:
  Worker(const std::string &name);

  // std::thread objects are not copiable so we don't want a worker object to be copied neither
  Worker(const Worker &) = delete;
  Worker &operator=(const Worker &) = delete;

  ~Worker();

  void start();
  bool wait();
  void notify();
  void stop();
  inline bool is_running() { return !finished; }

  inline const std::string get_name() const { return name; }
  inline const pid_t get_tid() const { return worker_tid; }
  inline void set_tid(const pid_t &tid) { worker_tid = tid; }

private:
  std::string name;

  bool finished;

  std::thread worker_thread;
  pid_t worker_tid;

  std::mutex worker_mutex;
  std::condition_variable worker_cv;

  void task();
  void handle_incoming_message(const message_model &new_message);
  void handle_system_message(const std::string& client_id, const std::string &command_message);

  inline pid_t sys_get_tid(void) { return syscall(SYS_gettid); }
  const application_id get_application_id(const std::string &s, const std::string app_id_separator = "^");

  std::shared_ptr<Logger> logger;
  std::shared_ptr<RemoteHandler> remote;
};

} // namespace agora

#endif // MARGOT_AGORA_WORKER_HPP
