/* core/papi_monitor.hpp
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

#ifndef MARGOT_PAPI_MONITOR_HDR
#define MARGOT_PAPI_MONITOR_HDR

#include <cstddef>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

#include <papi.h>

#include "margot/monitor.hpp"

namespace margot {

/**
 * @brief The list of event handled by the monitor
 */
enum class PapiEvent : int {
  L1_MISS = PAPI_L1_TCM,         // Level 1 cache misses
  L2_MISS = PAPI_L2_TCM,         // Level 2 cache misses
  L3_MISS = PAPI_L3_TCM,         // Level 3 cache misses
  INS_COMPLETED = PAPI_TOT_INS,  // Instructions completed
  NUM_BRANCH = PAPI_BR_INS,      // Number branch instruction
  NUM_LOAD = PAPI_LD_INS,        // Number load instruction
  NUM_STORE = PAPI_SR_INS,       // Number store instruction
  CYC_NO_ISSUE = PAPI_STL_ICY,   // Number of cycles with no istruction issue
  CYC_TOT = PAPI_TOT_CYC         // Total number of cycles
};

/**
 * @brief This monitor is a wrapper for the PAPI-framework
 * @details
 * The PAPI-framework enables a user to monitor performance events
 * regardless the used architecture. In order to see which events are supported
 * on your platform, use the tool "papi_avail".
 * All the monitors share the same PAPI event set, so starting a measure
 * has the side-effect to start the measures on all the used PAPI events.
 * If a call to the PAPI-framwork fails, is thrown an exception.
 *
 * @warning
 * Is not possible to start two partially overlapped measures, even if they are
 * interested in different events.
 *
 */
class PapiMonitor : public Monitor<long long int> {
 public:
  /**
   * @brief define the throughput_monitor type
   */
  using value_type = long long int;

  /****************************************************
   * Constructors and deconstructor
   ****************************************************/

  /**
   * @brief The trivial constructor
   */
  PapiMonitor(void);

  /**
   * @brief Construct the Papi monitor
   *
   * @param [in] event The interested event
   * @param [in] observation_size The size of the observation window
   *
   * @details
   * If the interested event is not available on the platform, an
   * exception is thrown.
   */
  PapiMonitor(const PapiEvent& event, const std::size_t observation_size = 1);

  /**
   * @brief Copy constructor
   *
   * @param [in] source The source papi_monitor
   */
  PapiMonitor(const PapiMonitor& source);

  /**
   * @brief Move constructor
   *
   * @param [in] source The source asrtm
   */
  PapiMonitor(PapiMonitor&& source);

  /**
   * @brief Assign operator (copy semantics)
   *
   * @param [in] source The source asrtm
   */
  PapiMonitor& operator=(const PapiMonitor& source) = default;

  /**
   * @brief Assign operator (move semantics)
   *
   * @param [in] source The source asrtm
   */
  PapiMonitor& operator=(PapiMonitor&& source) = default;

  /**
   * @brief Class destructor
   */
  ~PapiMonitor(void) { papi_interface_if::get_instance().erase(event); }

  /**
   * @brief Start the measure
   *
   * @details
   * Only the first call actually start the measures.
   * This means that all the calls of this method before
   * a monitor stops the measure, are useless.
   */
  inline void start(void) { papi_interface_if::get_instance().start(); }

  /**
   * @brief Stop the measure
   *
   * @details
   * Only the first call actually stop the measure,
   * however in order to retrieve the measure, every
   * monitor must call this method.
   */
  inline void stop(void) {
    papi_interface_if::get_instance().stop();
    this->push(papi_interface_if::get_instance().get_value(event));
  }

 private:
  /**
   * @breif This class represents an interface to PAPI
   *
   * @details
   * Since it is not possible to have overlapped measures, in the current
   * version this class represents a singleton interface to avoid to start
   * overlaped measures.
   */
  class papi_interface_if {
    /**
     * @brief Information about the monitored events
     */
    typedef struct {
      std::size_t counter;
      std::size_t position;
    } papi_event_t;

    /**
     * @brief typedef for storing information about the observed events
     */
    using event_info_t = std::unordered_map<int, papi_event_t>;

    /**
     * @brief typedef of the reading values
     */
    using event_values_t = std::vector<PapiMonitor::value_type>;

   public:
    /**
     * @brief Typedef for a pointer in the datastructure
     */
    using event_ptr_t = event_info_t::iterator;

    /****************************************************
     * Constructors and Deconstructors
     ****************************************************/

    /**
     * @brief Get the insance of the interface
     *
     * @return The reference of a PAPI interface
     */
    static papi_interface_if& get_instance(void) {
      static papi_interface_if instance;
      return instance;
    }

    /**
     * @brief Copy constructor
     *
     * @param [in] source The source sensor
     *
     * @details
     * Since there is at most one instance of the class, this method
     * is deleted.
     */
    papi_interface_if(const papi_interface_if& source) = delete;

    /**
     * @brief Move constructor
     *
     * @param [in] source The source interface
     *
     * @details
     * Since there is at most one instance of the class, this method
     * is deleted.
     */
    papi_interface_if(papi_interface_if&& source) = delete;

    /**
     * @brief Assign operator (copy semantics)
     *
     * @param [in] source The source interface
     *
     * @details
     * Since there is at most one instance of the class, this method
     * is deleted.
     */
    papi_interface_if& operator=(const papi_interface_if& source) = delete;

    /**
     * @brief Assign operator (copy semantics)
     *
     * @param [in] source The source interface
     *
     * @details
     * Since there is at most one instance of the class, this method
     * is deleted.
     */
    papi_interface_if& operator=(papi_interface_if&& source) = delete;

    /**
     * @brief The deconstructor
     */
    ~papi_interface_if(void);

    /****************************************************
     * Actual API implementation
     ****************************************************/

    /**
     * @brief Start the measures
     */
    void start(void);

    /**
     * @brief Stop the measures
     */
    void stop(void);

    /**
     * @brief Add an event to the observed event-set
     *
     * @param [in] event The target event
     *
     * @return A reference to the created element
     *
     * @details
     * All the registered events are inserted in the same shared event set.
     */
    event_ptr_t insert(const margot::PapiEvent& event);

    /**
     * @brief Remove an event from the observed event-set
     *
     * @param [in] event The target event
     *
     * @details
     * Only the last monitor that targets any given PAPI event actually removes
     * it from the observed event set
     */
    void erase(const event_ptr_t& event);

    /**
     * @brief Returns an invalid pointer
     *
     * @return An iterator to the end of the observed events
     */
    inline event_ptr_t get_nullpointer(void) { return observed_events.end(); }

    /**
     * @brief Return the observed data
     *
     * @param [in] event The target event
     *
     * @return The observed value
     */
    inline PapiMonitor::value_type get_value(const event_ptr_t& event) {
#ifndef NDEBUG

      if (event != observed_events.end()) {
        return static_cast<PapiMonitor::value_type>(values[event->second.position]);
      } else {
        return static_cast<PapiMonitor::value_type>(0);
      }

#else
      return static_cast<PapiMonitor::value_type>(values[event->second.position]);
#endif  // NDEBUG
    }

   private:
    /**
     * @brief Default constructor
     *
     * @details (use count)
     * In this way it is not possible to create more copies of the interface
     */
    papi_interface_if(void);

    /**
     * @brief The map that it is used to store the observed values
     *
     * @details
     * The key of the map is the event number in the PAPI interface, while
     * the value stored is a counter that counts the number of references
     * to that particular PAPI event.
     */
    event_info_t observed_events;

    /**
     * @brief State if a measure it is started
     */
    bool measuring;

    /**
     * @brief The last readed values
     *
     * @details
     * If the measure is started then this array contains all the starting
     * values for the observed PAPI events.
     * If the measure is not started then this array containts the result
     * of the observed metrics
     */
    event_values_t values;

    /**
     * @brief a mutex to protect read/write operations
     */
    std::mutex mutex;

    /**
     * @brief the PAPI event set
     */
    int event_set;
  };

  /**
   * @brief the monitored event
   */
  papi_interface_if::event_ptr_t event;
};

}  // namespace margot

#endif  // MARGOT_PAPI_MONITOR_HDR
