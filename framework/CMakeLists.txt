################################
#### Compile framework library
################################

# set the name of the final library
set( MARGOT_LIB_NAME "margot")


# set the headers of the margot library
set( HDR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include" )
set( MARGOT_HEADERS
	${HDR_PATH}/margot/basic_information_block.hpp
	${HDR_PATH}/margot/hash.hpp
	${HDR_PATH}/margot/operating_point.hpp
	${HDR_PATH}/margot/operating_point_segment.hpp
	${HDR_PATH}/margot/traits.hpp
	${HDR_PATH}/margot/knowledge_base.hpp
	${HDR_PATH}/margot/circular_buffer.hpp
	${HDR_PATH}/margot/statistical_provider.hpp
	${HDR_PATH}/margot/statistics.hpp
	${HDR_PATH}/margot/enums.hpp
	${HDR_PATH}/margot/field_adaptor.hpp
	${HDR_PATH}/margot/knowledge_adaptor.hpp
	${HDR_PATH}/margot/evaluator.hpp
	${HDR_PATH}/margot/view.hpp
	${HDR_PATH}/margot/monitor.hpp
	${HDR_PATH}/margot/rank.hpp
	${HDR_PATH}/margot/goal.hpp
	${HDR_PATH}/margot/constraint.hpp
	${HDR_PATH}/margot/state.hpp
	${HDR_PATH}/margot/asrtm.hpp
	${HDR_PATH}/margot/energy_monitor.hpp
	${HDR_PATH}/margot/frequency_monitor.hpp
	${HDR_PATH}/margot/memory_monitor.hpp
	${HDR_PATH}/margot/odroid_energy_monitor.hpp
	${HDR_PATH}/margot/odroid_power_monitor.hpp
	${HDR_PATH}/margot/process_cpu_usage_monitor.hpp
	${HDR_PATH}/margot/system_cpu_usage_monitor.hpp
	${HDR_PATH}/margot/throughput_monitor.hpp
	${HDR_PATH}/margot/time_monitor.hpp
	${HDR_PATH}/margot/da_asrtm.hpp
	${HDR_PATH}/margot/debug.hpp
	)
include_directories( ${HDR_PATH} )


# set the sources of the margot library
set( SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src" )
set( MARGOT_SOURCES
	${SRC_PATH}/energy_monitor
	${SRC_PATH}/frequency_monitor
	${SRC_PATH}/memory_monitor
	${SRC_PATH}/odroid_power_monitor
	${SRC_PATH}/odroid_energy_monitor
	${SRC_PATH}/process_cpu_usage_monitor
	${SRC_PATH}/system_cpu_usage_monitor
	${SRC_PATH}/throughput_monitor
	${SRC_PATH}/time_monitor
)


# add the collector monitor if enabled
if ( USE_COLLECTOR_MONITOR )
	list( APPEND MARGOT_HEADERS "${HDR_PATH}/margot/collector_monitor.hpp")
	list( APPEND MARGOT_SOURCES "${SRC_PATH}/collector_monitor.cc")
	list( APPEND MARGOT_DEPS_INCLUDE_DIRECTORIES "${COLLECTOR_INCLUDES}")
	include_directories( COLLECTOR_INCLUDES )
endif ( USE_COLLECTOR_MONITOR )


# add the papi monitor if enabled
if ( USE_PAPI_MONITOR )
	list( APPEND MARGOT_HEADERS "${HDR_PATH}/margot/papi_monitor.hpp")
	list( APPEND MARGOT_SOURCES "${SRC_PATH}/papi_monitor.cc")
	list( APPEND MARGOT_DEPS_INCLUDE_DIRECTORIES "${PAPI_INCLUDES}")
	include_directories( PAPI_INCLUDES )
endif ( USE_PAPI_MONITOR )


# add the temperature monitor if enabled
if ( USE_TEMPERATURE_MONITOR )
	list( APPEND MARGOT_HEADERS "${HDR_PATH}/margot/temperature_monitor.hpp")
	list( APPEND MARGOT_SOURCES "${SRC_PATH}/temperature_monitor.cc")
	list( APPEND MARGOT_DEPS_INCLUDE_DIRECTORIES "${SENSORS_INCLUDES}")
	include_directories( SENSORS_INCLUDES )
endif ( USE_TEMPERATURE_MONITOR )





# compile the library
if ( LIB_STATIC )
	add_library( ${MARGOT_LIB_NAME} STATIC ${MARGOT_HEADERS} ${MARGOT_SOURCES} )
else ( LIB_STATIC )
	add_library( ${MARGOT_LIB_NAME} SHARED ${MARGOT_HEADERS} ${MARGOT_SOURCES} )
endif ( LIB_STATIC )


# link the library also for the collector monitor
if ( USE_COLLECTOR_MONITOR )
	target_link_libraries( ${MARGOT_LIB_NAME} ${COLLECTOR_LIBRARIES} )
	list( APPEND MARGOT_LINK_DEPENDENCIES "${COLLECTOR_LIBRARIES}")
endif ( USE_COLLECTOR_MONITOR )

# link the library also for the papi monitor
if ( USE_PAPI_MONITOR )
	target_link_libraries( ${MARGOT_LIB_NAME} ${PAPI_LIBRARIES} )
	list( APPEND MARGOT_LINK_DEPENDENCIES "${PAPI_LIBRARIES}")
endif ( USE_PAPI_MONITOR )

# link the library also for the temperature monitor
if ( USE_TEMPERATURE_MONITOR )
	target_link_libraries( ${MARGOT_LIB_NAME} ${SENSORS_LIBRARIES} )
	list( APPEND MARGOT_LINK_DEPENDENCIES "${SENSORS_LIBRARIES}")
endif ( USE_TEMPERATURE_MONITOR )


# install the library
install( TARGETS ${MARGOT_NAME} DESTINATION lib )

# install the headers
install( FILES ${MARGOT_HEADERS} DESTINATION include/margot )


# install the margot cli stuff
install( FILES ${PROJECT_SOURCE_DIR}/margot_heel/margot_heel_cli/bin/margot_cli DESTINATION bin )
install( DIRECTORY ${PROJECT_SOURCE_DIR}/margot_heel/margot_heel_cli/margot_heel_cli DESTINATION bin )



################################
#### Build additional stuff
################################

add_subdirectory( config )

if ( WITH_TEST )
	add_subdirectory( test )
endif ( WITH_TEST )

if ( GEN_DOC )
	add_subdirectory( doc )
endif ( GEN_DOC )
