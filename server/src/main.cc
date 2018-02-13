#include <iostream>
#include <cstdlib>


#include "logger.hpp"
#include "paho_remote_implementation.hpp"
#include "virtual_channel.hpp"



int main( int argc, char* argv[] )
{
  margot::info("Application margot-agora started");

  // let's see if it actually work
  margot::VirtualChannel remote;
  remote.create_channel<margot::PahoClient>("127.0.0.1:1883", 2);
  remote.subscribe("margot");
  margot::message_t my_message;

  if (remote.recv_message(my_message))
  {
    std::cout << "YATTA! topic: " << my_message.topic << ", payload: " << my_message.payload << std::endl;
  }

  my_message.topic = "other";
  my_message.payload = "it works?";
  remote.send_message(my_message);

  return EXIT_SUCCESS;
}
