#ifndef LOGGER_CONFIGURATION_HPP
#define LOGGER_CONFIGURATION_HPP

#include <filesystem>
#include <string>

namespace agora {

enum class LoggerType { Console, File };

enum class LogLevel : uint8_t { DISABLED = 0, WARNING, INFO, PEDANTIC, DEBUG };
struct LoggerConfiguration
{
  LoggerConfiguration(LogLevel priority = LogLevel::DEBUG, LoggerType type = LoggerType::Console) : priority(priority), type(type) {}

  void set_file_logger_properties(const std::filesystem::path &log_file_path)
  {
    log_file = log_file_path;
  }

  LogLevel priority;
  LoggerType type;

  // file logger
  std::filesystem::path log_file;
};

} // namespace agora

#endif // LOGGER_CONFIGURATION_HPP
