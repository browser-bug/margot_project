#ifndef MARGOT_AGORA_LOGGER_HPP
#define MARGOT_AGORA_LOGGER_HPP

#include <chrono>
#include <cstdint>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <utility>

#include "agora/agora_properties.hpp"
#include "agora/logger_configuration.hpp"

namespace agora {

class Logger {
public:
  static std::unique_ptr<Logger> get_instance(const LoggerConfiguration &configuration);

  Logger(Logger const &) = delete;
  void operator=(Logger const &) = delete;

  template <typename... T> void debug(const T &...args)
  {
    if (static_cast<uint8_t>(LogLevel::DEBUG) <= static_cast<uint8_t>(configuration.priority))
    {
      std::string final_text = get_time_as_string() + " [DEBUG] " + get_log_text(args...);
      log(final_text);
    }
  }
  template <typename... T> void info(const T &...args)
  {
    if (static_cast<uint8_t>(LogLevel::INFO) <= static_cast<uint8_t>(configuration.priority))
    {
      std::string final_text = get_time_as_string() + " [INFO] " + get_log_text(args...);
      log(final_text);
    }
  }
  template <typename... T> void warning(const T &...args)
  {
    if (static_cast<uint8_t>(LogLevel::WARNING) <= static_cast<uint8_t>(configuration.priority))
    {
      std::string final_text = get_time_as_string() + " [WARNING] " + get_log_text(args...);
      log(final_text);
    }
  }
  template <typename... T> void pedantic(const T &...args)
  {
    if (static_cast<uint8_t>(LogLevel::PEDANTIC) <= static_cast<uint8_t>(configuration.priority))
    {
      std::string final_text = get_time_as_string() + " [PEDANTIC] " + get_log_text(args...);
      log(final_text);
    }
  }

  inline void set_log_filter(const LogLevel &new_log_level) { configuration.priority = new_log_level; }

protected:
  Logger(const LoggerConfiguration &configuration) : configuration(configuration){};
  const std::string get_time_as_string() const;

  template <typename... T> const std::string get_log_text(T &...args) const
  {
    std::ostringstream log_text;
    ((log_text << std::forward<T>(args)), ...);
    return log_text.str();
  }

  // internal log function that needs to be specified by every type of Logger
  virtual void log(const std::string &text) = 0;

  LoggerConfiguration configuration;
};

} // namespace agora

#endif // MARGOT_AGORA_LOGGER_HPP
