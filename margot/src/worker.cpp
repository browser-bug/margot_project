#include <iostream>
#include <string>

#include "agora/application_manager.hpp"
#include "agora/logger.hpp"
#include "agora/worker.hpp"

namespace agora {

Worker::Worker(const std::string &name) : name(name), finished(false), worker_tid(0)
{
  ApplicationManager &am = ApplicationManager::get_instance();
  logger = am.get_logger();
  remote = am.get_remote_handler();
}

Worker::~Worker() { stop(); }

void Worker::start()
{
  std::unique_lock<std::mutex> lock(worker_mutex);

  logger->debug("Worker thread [", get_name(), "] is starting.");

  worker_thread = std::thread(&Worker::task, this);

  // since we always exprect all the workers to terminate on agora termination
  // there's no need to sync on termination and so we can detach.
  worker_thread.detach();
}

void Worker::stop()
{
  logger->debug("Thread ", get_tid(), " on retirement.");

  std::unique_lock<std::mutex> lock(worker_mutex);
  if (finished)
  {
    logger->warning("Thread ", get_tid(), "is already terminated.");
    return;
  }
  finished = true;

  worker_cv.notify_all();

  assert(get_tid() != 0); // checking that we're not terminating a thread that has not even started
  ::kill(get_tid(), SIGUSR1);

  logger->debug("Worker thread [", get_name(), "] has terminated succesfully.");
}

bool Worker::wait()
{
  std::unique_lock<std::mutex> lock(worker_mutex);

  logger->debug("Waiting on thread ", get_tid());
  worker_cv.wait(lock);
  return !finished;
}

void Worker::notify()
{
  std::unique_lock<std::mutex> lock(worker_mutex);

  logger->debug("Thread ", get_tid(), " notifying.");
  worker_cv.notify_all();
}

void Worker::task()
{
  set_tid(sys_get_tid());

  // notify that we are a new thread
  logger->debug("Thread ", get_tid(), " on duty.");

  // assuming that there is plenty of work for everybody
  while (!finished)
  {
    // declaring the new message
    message_model new_incoming_message;

    // this is a blocking call
    if (!remote->recv_message(new_incoming_message))
    {
      stop(); // there is no more work available
      continue;
    }

    // otherwise process the incoming message
    handle_incoming_message(new_incoming_message);
  }
}

void Worker::handle_system_message(const std::string &client_id, const std::string &command_message)
{
  // system message payload is split like this: type[@optional_message]+
  const auto command_msg_tokens = tokenize(command_message, "@");
  if(command_msg_tokens.empty())
  {
    logger->warning("Received system message with invalid payload format.");
    return;
  }
  const auto command_type = command_msg_tokens.front();

  switch (resolve_system_command_type(command_type))
  {
  case AgoraSystemCommandType::Shutdown:
    stop();
    break;
  case AgoraSystemCommandType::TestConnection: {
    const auto client_msg = (command_msg_tokens.size() == 2) ? command_msg_tokens.at(1) : "";

    logger->info("TestConnection: server just received a new message {", client_msg, "} from client [", client_id, "].");
    remote->send_message({MESSAGE_HEADER + "/" + client_id + "/test", "Hello from server " + client_id});
    break;
  }
  case AgoraSystemCommandType::Invalid_Command:
    logger->warning("Invalid system command: ", command_message);
    break;
  }
}

void Worker::handle_incoming_message(const message_model &new_message)
{
  ApplicationManager &am = ApplicationManager::get_instance();

  // get the last part of the topic to understand what the message is about
  const auto topic_tokens = tokenize(new_message.topic, "/");
  if(topic_tokens.size() < 2)
  {
    logger->warning("Received message with invalid topic format.");
    return;
  }
  const auto client_id = topic_tokens.end()[-1];
  const auto message_type = topic_tokens.end()[-2];

  switch (resolve_message_type(message_type))
  {
  case AgoraMessageType::System: {
    const auto system_command_message = new_message.payload;
    handle_system_message(client_id, system_command_message);
    break;
  }
  case AgoraMessageType::Welcome: {
    const auto app_id = get_application_id(topic_tokens.at(1));

    const auto application_info = new_message.payload;
    const auto application_handler = am.get_application_handler(app_id);

    logger->pedantic("Thread ", get_tid(), ": new client \"", client_id, "\" for application \"", app_id.str(), "\".");

    application_handler->welcome_client(client_id, application_info);
    break;
  }
  case AgoraMessageType::Kia: {
    const auto app_id = get_application_id(topic_tokens.at(1));

    const auto reason = new_message.payload;
    const auto application_handler = am.get_application_handler(app_id);

    logger->pedantic("Thread ", get_tid(), ": connection lost with client \"", client_id, "\" for application \"", app_id.str(),
                     "\". Reason: ", reason);

    //application_handler->bye_client(client_id);
    break;
  }
  case AgoraMessageType::Observation: {
    const auto app_id = get_application_id(topic_tokens.at(1));

    const auto payload_tokens = tokenize(new_message.payload, "@");
    if (payload_tokens.size() != 4)
    {
      logger->warning("Received observation message with invalid payload format.");
      return;
    }

    const auto client_id = payload_tokens.at(0);
    const auto timestamp_sec = std::stol(payload_tokens.at(1));
    const auto timestamp_ns = std::stol(payload_tokens.at(2));
    const auto observation = payload_tokens.at(3);

    const auto application_handler = am.get_application_handler(app_id);

    logger->pedantic("Thread ", get_tid(), ": received a new observation for application \"", app_id.str(), "\" {", observation, "}.");

    application_handler->process_observation(client_id, timestamp_sec, timestamp_ns, observation);
    break;
  }
  case AgoraMessageType::Error: {
    const auto error_msg = new_message.payload;
    logger->warning("Error message -> ", error_msg, " Ignoring the message");
    break;
  }
  case AgoraMessageType::Invalid_Message:
    logger->warning("Invalid message type received: ", message_type);
  }
}

const application_id Worker::get_application_id(const std::string &s, const std::string app_id_separator)
{
  // TODO: we're splitting the application_id based on an hardcoded character instead of using topic division
  // (app_name/block_name/version), is this still ok?
  const auto app_id_tokens = tokenize(s, app_id_separator);
  return application_id(app_id_tokens.at(0), app_id_tokens.at(1), app_id_tokens.at(2));
}

} // namespace agora
