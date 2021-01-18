#ifndef APPLICATION_MANAGER_HPP
#define APPLICATION_MANAGER_HPP

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "agora/application_handler.hpp"
#include "agora/logger.hpp"
#include "agora/remote_handler.hpp"
#include "agora/fs_handler.hpp"
#include "agora/launcher.hpp"

namespace agora {

class ApplicationManager {
public:
  static ApplicationManager &get_instance()
  {
    static ApplicationManager am;
    return am;
  }

  // setup functions for each internal entity
  void setup_logger(const LoggerConfiguration &config) {
    std::lock_guard<std::mutex> lock(global_mutex);
    logger = Logger::get_instance(config);
  }
  void setup_remote_handler(const RemoteConfiguration &config) {
    std::lock_guard<std::mutex> lock(global_mutex);
    remote = RemoteHandler::get_instance(config);
  }

  inline void set_filesystem_configuration(const FsConfiguration &config) { fs_configuration = config; }
  inline void set_launcher_configuration(const LauncherConfiguration &config) { launcher_configuration = config; }

  inline const std::shared_ptr<Logger> get_logger() const
  {
    return logger;
  }

  inline const std::shared_ptr<RemoteHandler> get_remote_handler() const
  {
    return remote;
  }

  inline std::shared_ptr<RemoteApplicationHandler> get_application_handler(const application_id &app_handler_id)
  {
    std::lock_guard<std::mutex> lock(global_mutex);
    auto iterator = apps.find(app_handler_id.str());

    if (iterator == apps.end())
    {
      auto &&application_ptr = std::make_shared<RemoteApplicationHandler>(app_handler_id, fs_configuration, launcher_configuration);
      const auto result_pair = apps.emplace(app_handler_id.str(), application_ptr);

      logger->debug("Creating a new application handler with ID [", app_handler_id.str(), "].");
      return result_pair.first->second;
    }

    return iterator->second;
  }

  inline void remove_application_handler(const application_id &app_handler_id)
  {
    std::lock_guard<std::mutex> lock(global_mutex);
    auto iterator = apps.find(app_handler_id.str());

    if (iterator != apps.end())
    {
      apps.erase(iterator);
    }
    else{
      logger->warning("Couldn't remove application handler: ID not found [", app_handler_id.str(), "].");
    }
  }

private:
  ApplicationManager() {}
  std::mutex global_mutex;
  std::unordered_map<std::string, std::shared_ptr<RemoteApplicationHandler>> apps;

  std::shared_ptr<Logger> logger;
  std::shared_ptr<RemoteHandler> remote;

  FsConfiguration fs_configuration;
  LauncherConfiguration launcher_configuration;
};

} // namespace agora

#endif // APPLICATION_MANAGER_HPP
