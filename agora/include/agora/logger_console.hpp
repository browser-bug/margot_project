#ifndef LOGGER_CONSOLE_HPP
#define LOGGER_CONSOLE_HPP

#include "agora/logger.hpp"

namespace agora {

class ConsoleLogger : public Logger {

public:
  ConsoleLogger(const LoggerConfiguration &configuration) : Logger(configuration){};

private:
  // std::cout is thread safe but we need to aboid multiple threads message interleaving on one line.
  // So only call the stream output operator << once.
  void log(const std::string &text) override
  {
    std::cout << text + "\n";
    std::cout.flush();
  }
};

} // namespace agora

#endif // LOGGER_CONSOLE_HPP
