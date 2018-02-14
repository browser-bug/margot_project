#include <iostream>
#include <cstdlib>


#include "logger.hpp"
#include "paho_remote_implementation.hpp"
#include "virtual_channel.hpp"
#include "threadpool.hpp"
#include "worker.hpp"



int main( int argc, char* argv[] )
{

  // create a virtual channel to communicate with the applications
  margot::info("Agora main: bootstrap step 1: estabilish a connection with broker");

  // establish a connection with broker
  margot::VirtualChannel remote;
  remote.create_channel<margot::PahoClient>("127.0.0.1:1883", 2);

  // subscribes to the initial set of topics
  remote.subscribe("margot/welcome");
  remote.subscribe("margot/system");

  // start the thread pool of worker that manage the applications
  margot::info("Agora main: bootstrap step 2: hiring the oompa loompas");
  margot::ThreadPool<margot::Worker> workers(remote, 3);


  // wain until the workers have done
  margot::info("Agora main: bootstrap complete, waiting for workers to finish");
  workers.wait_workers();


  // ok, the whole server is down, time to go out of business
  margot::info("Agora main: all the workers have joined me, farewell my friend");

  return EXIT_SUCCESS;
}
