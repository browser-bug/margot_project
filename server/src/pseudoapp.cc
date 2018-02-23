/* agora/pseudoapp.cc
 * Copyright (C) 2018 Davide Gadioli
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 * USA
 */


#include <cstdlib>
#include <chrono>

#include "application_stub.hpp"
#include "threadpool.hpp"


int main( int argc, char* argv[] )
{

  // run n istances of the application for m seconds
  margot::Application application_stub;
  application_stub(std::chrono::seconds(20));
  //margot::ThreadPool workers(3, application_stub, std::chrono::seconds(5));

  return EXIT_SUCCESS;
}
