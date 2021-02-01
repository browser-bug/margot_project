#ifndef LOGGER_FILE_HPP
#define LOGGER_FILE_HPP

#include "agora/logger.hpp"

namespace agora {

class FileLogger : public Logger {

public:
  FileLogger(const LoggerConfiguration &configuration) : Logger(configuration)
  {
    log_file.open(configuration.log_file, std::ios::out | std::ios::trunc);
  };

  ~FileLogger()
  {
    if (log_file.is_open())
      log_file.close();
  };

private:
  std::ofstream log_file;
  std::mutex logger_mutex;

  void log(const std::string &text) override
  {
    std::lock_guard<std::mutex> lock(logger_mutex);
    log_file << text << std::endl;
  }
};

} // namespace agora

#endif // LOGGER_FILE_HPP
