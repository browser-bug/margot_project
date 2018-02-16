#include <iostream>
#include <cstdlib>
#include <chrono>

#include "application_stub.hpp"


int main( int argc, char* argv[] )
{

  // run the application stub for 5 secs
  margot::Application local_application;
  local_application(std::chrono::seconds(5));

  return EXIT_SUCCESS;
}
