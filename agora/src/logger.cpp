#include <cstdarg>

#include "agora/logger.hpp"
#include "agora/logger_console.hpp"
#include "agora/logger_file.hpp"

using namespace agora;

std::unique_ptr<Logger> Logger::get_instance(const LoggerConfiguration &configuration)
{
  std::unique_ptr<Logger> logger;

  switch (configuration.type)
  {
  case LoggerType::Console:
    logger = std::make_unique<ConsoleLogger>(configuration);
    break;
  case LoggerType::File:
    logger = std::make_unique<FileLogger>(configuration);
    break;
  default:
    logger = std::make_unique<ConsoleLogger>(configuration);
    logger->warning("Unknown logger type. Setting default console logger.");
  }

  return logger;
}

const std::string Logger::get_time_as_string() const
{
  const auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  std::ostringstream oss;
  oss << std::put_time(std::localtime(&now), "%F %T");
  return oss.str();
}
