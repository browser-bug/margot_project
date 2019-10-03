/* core/papi_monitor.cc
 * Copyright (C) 2017 Davide Gadioli
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

#include <stdio.h>
#include <cstring>
#include <map>
#include <mutex>
#include <stdexcept>
#include <vector>

#include <margot/papi_monitor.hpp>

namespace margot {
// Helper function
void handle_error(int retval) {
  throw std::runtime_error("PAPI exception \"" + std::string(PAPI_strerror(retval)) + "\"");
}

PapiMonitor::PapiMonitor(void) : Monitor() { event = papi_interface_if::get_instance().get_nullpointer(); }

PapiMonitor::PapiMonitor(const PapiEvent& event, const std::size_t observation_size)
    : Monitor(observation_size) {
  this->event = papi_interface_if::get_instance().insert(event);
}

PapiMonitor::PapiMonitor(const PapiMonitor& source) {
  // register the new reference
  this->event = papi_interface_if::get_instance().insert(static_cast<PapiEvent>(source.event->first));

  // assign the buffer pointer (with copy semantic)
  buffer = source.buffer;
}

PapiMonitor::PapiMonitor(PapiMonitor&& source) {
  // register the new reference
  this->event = papi_interface_if::get_instance().insert(static_cast<PapiEvent>(source.event->first));

  // assign the buffer pointer (with move semantic)
  buffer = std::move(source.buffer);
}

// CONSTRUCTOR IF -> Initialize everything
PapiMonitor::papi_interface_if::papi_interface_if(void) {
  // initialize the data structure
  int retval = PAPI_library_init(PAPI_VER_CURRENT);

  if (retval != PAPI_VER_CURRENT) {
    throw std::runtime_error("PAPI init ERROR!");
  }

  // create the event set
  retval = PAPI_create_eventset(&event_set);

  if (retval != PAPI_OK) {
    handle_error(retval);
  }

  // initialize the measure flag
  measuring = false;
}

// DECONSTRUCTOR IF -> Cleanup & destroy everything related to PAPI
PapiMonitor::papi_interface_if::~papi_interface_if(void) {
  // check if we are running some measures
  if (measuring) {
    // if so, stop it
    int retval = PAPI_stop(event_set, values.data());

    if (retval != PAPI_OK) {
      handle_error(retval);
    }
  }

  // cleanup the eventset
  int retval = PAPI_cleanup_eventset(event_set);

  if (retval != PAPI_OK) {
    handle_error(retval);
  }

  // destroy the eventset
  retval = PAPI_destroy_eventset(&event_set);

  if (retval != PAPI_OK) {
    handle_error(retval);
  }

  // shutdown the PAPI framework
  PAPI_shutdown();
}

PapiMonitor::papi_interface_if::event_ptr_t PapiMonitor::papi_interface_if::insert(
    const margot::PapiEvent& event) {
  // acquire the lock
  std::lock_guard<std::mutex> lock(mutex);

  // make sure that we are not measuring
  if (!measuring) {
    // Query if the event is available
    int retval = PAPI_query_event(static_cast<int>(event));

    if (retval != PAPI_OK) {
      handle_error(retval);
    }

    // look for the target event
    event_ptr_t element = observed_events.find(static_cast<int>(event));

    // If it is not already observed
    if (element != observed_events.end()) {
      // get the size of the bserved events
      const std::size_t pos = observed_events.size();

      // add it to the observed set
      std::pair<event_info_t::iterator, bool> ret;
      ret = observed_events.insert({static_cast<int>(event), {0, pos}});
      element = ret.first;

      // Add the event to the event set
      retval = PAPI_add_event(event_set, static_cast<int>(event));

      if (retval != PAPI_OK) {
        handle_error(retval);
      }

      // resize the array
      values.resize(pos + 1);

      // initialize it to 0
      values[pos] = 0;
    } else {
      // Otherwise increment the counter
      element->second.counter += 1;
    }

    // return a reference to the element
    return element;
  }

  // return a reference to an invalid pointer
  return observed_events.end();
}

void PapiMonitor::papi_interface_if::erase(const event_ptr_t& event) {
  // acquire the lock
  std::lock_guard<std::mutex> lock(mutex);

  // check if we are not actually measuring some metrics
  if (!measuring) {
    // check if the event is a valid pointer reference
    if (event != observed_events.end()) {
      // check if we there is only one monitor interested to the event
      if (event->second.counter > 1) {
        // decrease it
        event->second.counter += 1;
      } else {
        // get the position of the erased element
        const std::size_t pos_elem = event->second.position;

        // remove the event form the observed_events
        observed_events.erase(event);

        // resize the values
        values.resize(observed_events.size());

        // decrease the values of the other events
        for (auto& event : observed_events) {
          // decrease the position if needed
          if (event.second.position > pos_elem) {
            event.second.position -= 1;
          }
        }
      }
    }
  }
}

void PapiMonitor::papi_interface_if::start(void) {
  // lock the operation
  std::lock_guard<std::mutex> lock(mutex);

  // check if we have alredy started the measure
  if (measuring) {
    return;
  }

  // otherwise, start the measure
  int retval = PAPI_start(event_set);

  if (retval != PAPI_OK) {
    handle_error(retval);
  }

  // update the monitor status
  measuring = true;
}

void PapiMonitor::papi_interface_if::stop(void) {
  // lock the operation
  std::lock_guard<std::mutex> lock(mutex);

  // check if we have alredy stopped the measure
  if (measuring) {
    // stop the measure
    int retval = PAPI_stop(event_set, values.data());

    if (retval != PAPI_OK) {
      handle_error(retval);
    }

    // update the monitor status
    measuring = false;
  }
}

}  // namespace margot
