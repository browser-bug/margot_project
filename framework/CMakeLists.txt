################################
#### Compile framework library
################################

# set the headers of the margot library
set( MARGOT_HEADERS
	${MARGOT_HDR_PATH}/margot/basic_information_block.hpp
	${MARGOT_HDR_PATH}/margot/hash.hpp
	${MARGOT_HDR_PATH}/margot/operating_point.hpp
	${MARGOT_HDR_PATH}/margot/operating_point_segment.hpp
	${MARGOT_HDR_PATH}/margot/traits.hpp
	${MARGOT_HDR_PATH}/margot/knowledge_base.hpp
	${MARGOT_HDR_PATH}/margot/circular_buffer.hpp
	${MARGOT_HDR_PATH}/margot/statistical_provider.hpp
	${MARGOT_HDR_PATH}/margot/statistics.hpp
	${MARGOT_HDR_PATH}/margot/enums.hpp
	${MARGOT_HDR_PATH}/margot/field_adaptor.hpp
	${MARGOT_HDR_PATH}/margot/knowledge_adaptor.hpp
	${MARGOT_HDR_PATH}/margot/evaluator.hpp
	${MARGOT_HDR_PATH}/margot/view.hpp
	${MARGOT_HDR_PATH}/margot/monitor.hpp
	${MARGOT_HDR_PATH}/margot/rank.hpp
	${MARGOT_HDR_PATH}/margot/goal.hpp
	${MARGOT_HDR_PATH}/margot/constraint.hpp
	${MARGOT_HDR_PATH}/margot/state.hpp
	${MARGOT_HDR_PATH}/margot/asrtm.hpp
	${MARGOT_HDR_PATH}/margot/energy_monitor.hpp
	${MARGOT_HDR_PATH}/margot/frequency_monitor.hpp
	${MARGOT_HDR_PATH}/margot/memory_monitor.hpp
	${MARGOT_HDR_PATH}/margot/odroid_energy_monitor.hpp
	${MARGOT_HDR_PATH}/margot/odroid_power_monitor.hpp
	${MARGOT_HDR_PATH}/margot/process_cpu_usage_monitor.hpp
	${MARGOT_HDR_PATH}/margot/system_cpu_usage_monitor.hpp
	${MARGOT_HDR_PATH}/margot/throughput_monitor.hpp
	${MARGOT_HDR_PATH}/margot/time_monitor.hpp
	${MARGOT_HDR_PATH}/margot/da_asrtm.hpp
	${MARGOT_HDR_PATH}/margot/debug.hpp
	${MARGOT_HDR_PATH}/margot/data_features.hpp
	)
set( AGORA_HEADERS
  ${MARGOT_HDR_PATH}/agora/application_handler.hpp
  ${MARGOT_HDR_PATH}/agora/cassandra_fs_implementation.hpp
  ${MARGOT_HDR_PATH}/agora/common_objects.hpp
  ${MARGOT_HDR_PATH}/agora/doe.hpp
  ${MARGOT_HDR_PATH}/agora/fs_handler.hpp
  ${MARGOT_HDR_PATH}/agora/global_view.hpp
	${MARGOT_HDR_PATH}/agora/logger.hpp
  ${MARGOT_HDR_PATH}/agora/model_generator.hpp
  ${MARGOT_HDR_PATH}/agora/paho_remote_implementation.hpp
  ${MARGOT_HDR_PATH}/agora/remote_handler.hpp
	${MARGOT_HDR_PATH}/agora/safe_queue.hpp
  ${MARGOT_HDR_PATH}/agora/threadpool.hpp
	${MARGOT_HDR_PATH}/agora/virtual_channel.hpp
	${MARGOT_HDR_PATH}/agora/virtual_fs.hpp
	${MARGOT_HDR_PATH}/agora/virtual_io.hpp
  ${MARGOT_HDR_PATH}/agora/worker.hpp
)

set( BEHOLODER_HEADERS
  ${MARGOT_HDR_PATH}/beholder/worker_beholder.hpp
  ${MARGOT_HDR_PATH}/beholder/global_view_beholder.hpp
  ${MARGOT_HDR_PATH}/beholder/application_handler_beholder.hpp
	${MARGOT_HDR_PATH}/beholder/parameters_beholder.hpp
	${MARGOT_HDR_PATH}/beholder/observation_data.hpp
	${MARGOT_HDR_PATH}/beholder/ici_test_data.hpp
	${MARGOT_HDR_PATH}/beholder/ici_cdt.hpp
	${MARGOT_HDR_PATH}/beholder/hypothesis_test.hpp
	${MARGOT_HDR_PATH}/beholder/hypothesis_test_data.hpp
)


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
set( AGORA_SOURCES
	${SRC_PATH}/logger
	${SRC_PATH}/paho_remote_implementation
	${SRC_PATH}/cassandra_fs_implementation
	${SRC_PATH}/application_handler
	${SRC_PATH}/worker
	${SRC_PATH}/virtual_io
	${SRC_PATH}/global_view
	${SRC_PATH}/model_generator
)
set( BEHOLDER_SOURCES
	${SRC_PATH}/worker_beholder
	${SRC_PATH}/global_view_beholder
	${SRC_PATH}/application_handler_beholder
	${SRC_PATH}/parameters_beholder
	${SRC_PATH}/ici_cdt
	${SRC_PATH}/hypothesis_test
)

# add the collector monitor if enabled
if ( USE_COLLECTOR_MONITOR )
	list( APPEND MARGOT_HEADERS "${MARGOT_HDR_PATH}/margot/collector_monitor.hpp")
	list( APPEND MARGOT_SOURCES "${SRC_PATH}/collector_monitor.cc")
	list( APPEND MARGOT_DEPS_INCLUDE_DIRECTORIES "${COLLECTOR_INCLUDES}")
	include_directories( COLLECTOR_INCLUDES )
endif ( USE_COLLECTOR_MONITOR )


# add the papi monitor if enabled
if ( USE_PAPI_MONITOR )
	list( APPEND MARGOT_HEADERS "${MARGOT_HDR_PATH}/margot/papi_monitor.hpp")
	list( APPEND MARGOT_SOURCES "${SRC_PATH}/papi_monitor.cc")
	list( APPEND MARGOT_DEPS_INCLUDE_DIRECTORIES "${PAPI_INCLUDES}")
	include_directories( PAPI_INCLUDES )
endif ( USE_PAPI_MONITOR )


# add the temperature monitor if enabled
if ( USE_TEMPERATURE_MONITOR )
	list( APPEND MARGOT_HEADERS "${MARGOT_HDR_PATH}/margot/temperature_monitor.hpp")
	list( APPEND MARGOT_SOURCES "${SRC_PATH}/temperature_monitor.cc")
	list( APPEND MARGOT_DEPS_INCLUDE_DIRECTORIES "${SENSORS_INCLUDES}")
	include_directories( SENSORS_INCLUDES )
endif ( USE_TEMPERATURE_MONITOR )

# add header dependency if using agora
if ( WITH_AGORA )
	list( APPEND MARGOT_DEPS_INCLUDE_DIRECTORIES "${MQTT_INCLUDES}")
	list( APPEND MARGOT_DEPS_INCLUDE_DIRECTORIES "${CASSANDRA_INCLUDES}")
endif ( WITH_AGORA )


# create the headers and sources list for the final margot lib
if ( WITH_AGORA )
	set( MARGOT_FINAL_HEADERS ${MARGOT_HEADERS} ${AGORA_HEADERS} ${BEHOLDER_HEADERS})
	set( MARGOT_FINAL_SOURCES ${MARGOT_SOURCES} ${AGORA_SOURCES} ${BEHOLDER_SOURCES})
else ( WITH_AGORA )
	set( MARGOT_FINAL_HEADERS ${MARGOT_HEADERS} )
	set( MARGOT_FINAL_SOURCES ${MARGOT_SOURCES} )
endif ( WITH_AGORA )


# compile the library
if ( LIB_STATIC )
	add_library( ${MARGOT_LIB_NAME} STATIC ${MARGOT_FINAL_HEADERS} ${MARGOT_FINAL_SOURCES} )
else ( LIB_STATIC )
	add_library( ${MARGOT_LIB_NAME} SHARED ${MARGOT_FINAL_HEADERS} ${MARGOT_FINAL_SOURCES} )
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

# add lib dependency if using agora
if ( WITH_AGORA )
	list( APPEND MARGOT_LINK_DEPENDENCIES "${MQTT_LIBRARIES}")
	list( APPEND MARGOT_LINK_DEPENDENCIES "${CASSANDRA_LIBRARIES}")
endif ( WITH_AGORA )

# install the library
install( TARGETS ${MARGOT_LIB_NAME} DESTINATION lib )

# install the headers
install( FILES ${MARGOT_HEADERS} DESTINATION include/margot )
install( FILES ${MARGOT_CONFIG_FILE_PATH}/margot/margot_config.hpp DESTINATION include/margot )
if ( WITH_AGORA )
	install( FILES ${AGORA_HEADERS} DESTINATION include/agora )
	install( FILES ${BEHOLDER_HEADERS} DESTINATION include/beholder )
endif ( WITH_AGORA )


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
