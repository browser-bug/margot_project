###############################################
##      Compiling the framework module       ##
###############################################



# -------- Defining the source/header files
set( MONITOR_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src )


set( MONITOR_HEADERS  ${MONITOR_HDR_PATH}/margot/monitor.hpp
                      ${MONITOR_HDR_PATH}/margot/goal.hpp
                      ${MONITOR_HDR_PATH}/margot/time_monitor.hpp
                      ${MONITOR_HDR_PATH}/margot/throughput_monitor.hpp
                      ${MONITOR_HDR_PATH}/margot/system_cpu_usage_monitor.hpp
                      ${MONITOR_HDR_PATH}/margot/process_cpu_usage_monitor.hpp
                      ${MONITOR_HDR_PATH}/margot/memory_monitor.hpp
                      ${MONITOR_HDR_PATH}/margot/frequency_monitor.hpp
                      ${MONITOR_HDR_PATH}/margot/energy_monitor.hpp
                      ${MONITOR_HDR_PATH}/margot/odroid_power_monitor.hpp
                      ${MONITOR_HDR_PATH}/margot/odroid_energy_monitor.hpp
                      ${CONFIG_FILE}
   )
set( MONITOR_SOURCES  ${MONITOR_SRC_PATH}/goal
                      ${MONITOR_SRC_PATH}/time_monitor
                      ${MONITOR_SRC_PATH}/throughput_monitor
                      ${MONITOR_SRC_PATH}/system_cpu_usage_monitor
                      ${MONITOR_SRC_PATH}/process_cpu_usage_monitor
                      ${MONITOR_SRC_PATH}/memory_monitor
                      ${MONITOR_SRC_PATH}/frequency_monitor
                      ${MONITOR_SRC_PATH}/energy_monitor
                      ${MONITOR_SRC_PATH}/odroid_power_monitor
                      ${MONITOR_SRC_PATH}/odroid_energy_monitor
   )


# check if we are able to compile a PAPI monitor
if( HAVE_PAPI )
        set( MONITOR_HEADERS ${MONITOR_HEADERS} ${MONITOR_HDR_PATH}/margot/papi_monitor.hpp )
        set( MONITOR_SOURCES ${MONITOR_SOURCES} ${MONITOR_SRC_PATH}/papi_monitor )
endif( HAVE_PAPI )


# check if we are able to compile the SENSORS monitor
if ( HAVE_SENSORS )
        set( MONITOR_HEADERS ${MONITOR_HEADERS} ${MONITOR_HDR_PATH}/margot/temperature_monitor.hpp )
        set( MONITOR_SOURCES ${MONITOR_SOURCES} ${MONITOR_SRC_PATH}/temperature_monitor )
endif( HAVE_SENSORS )


# check if we are able to compile the COLLECTOR monitor
if ( HAVE_COLLECTOR )
        set( MONITOR_HEADERS ${MONITOR_HEADERS} ${MONITOR_HDR_PATH}/margot/collector_monitor.hpp )
        set( MONITOR_SOURCES ${MONITOR_SOURCES} ${MONITOR_SRC_PATH}/collector_monitor )
endif( HAVE_COLLECTOR )




# --------  Build the module
include_directories(${MONITOR_HDR_PATH})
include_directories(${CONFIGURATION_HDR_PATH})
if( HAVE_PAPI )
        include_directories(${PAPI_INCLUDES})
endif( HAVE_PAPI )
if( HAVE_SENSORS )
        include_directories(${SENSORS_INCLUDES})
endif( HAVE_SENSORS )
if( HAVE_COLLECTOR )
        include_directories(${COLLECTOR_INCLUDES})
endif( HAVE_COLLECTOR )



if   ( LIB_STATIC )
        add_library( ${MONITOR_LIB_NAME} STATIC ${MONITOR_SOURCES} ${MONITOR_HEADERS} )
else ( LIB_STATIC )
        add_library( ${MONITOR_LIB_NAME} SHARED ${MONITOR_SOURCES} ${MONITOR_HEADERS} )
endif( LIB_STATIC )

target_link_libraries( ${MONITOR_LIB_NAME} m)
if( HAVE_PAPI )
        target_link_libraries(${MONITOR_LIB_NAME} ${PAPI_LIBRARIES})
endif( HAVE_PAPI )
if( HAVE_SENSORS )
        target_link_libraries(${MONITOR_LIB_NAME} ${SENSORS_LIBRARIES})
endif( HAVE_SENSORS )
if( HAVE_COLLECTOR )
        target_link_libraries(${MONITOR_LIB_NAME} ${COLLECTOR_LIBRARIES})
endif( HAVE_COLLECTOR )



# ------ Install the module
install( TARGETS ${MONITOR_LIB_NAME} DESTINATION lib )
install( FILES ${MONITOR_HEADERS} DESTINATION include/margot )




###############################################
##      Adding the headers to the test       ##
###############################################

# this variable it is used to build the test suite
set ( MONITOR_TEST_HEADERS_TEMP ${CMAKE_CURRENT_SOURCE_DIR}/test/monitor_basic.hpp
                                ${CMAKE_CURRENT_SOURCE_DIR}/test/goal.hpp
                                ${CMAKE_CURRENT_SOURCE_DIR}/test/monitor_measure.hpp
    )

# optionally add the papi monitor header
if( HAVE_PAPI )
       set ( MONITOR_TEST_HEADERS_TEMP ${MONITOR_TEST_HEADERS_TEMP} ${CMAKE_CURRENT_SOURCE_DIR}/test/monitor_papi.hpp )
endif( HAVE_PAPI )

# optionally add the sensors monitor header
if( HAVE_SENSORS )
       set ( MONITOR_TEST_HEADERS_TEMP ${MONITOR_TEST_HEADERS_TEMP} ${CMAKE_CURRENT_SOURCE_DIR}/test/monitor_sensors.hpp )
endif( HAVE_SENSORS )

# optionally add the collector monitor header
if( HAVE_COLLECTOR )
       set ( MONITOR_TEST_HEADERS_TEMP ${MONITOR_TEST_HEADERS_TEMP} ${CMAKE_CURRENT_SOURCE_DIR}/test/monitor_collector.hpp )
endif( HAVE_COLLECTOR )

# make it official
set( MONITOR_TEST_HEADERS ${MONITOR_TEST_HEADERS_TEMP} PARENT_SCOPE )
