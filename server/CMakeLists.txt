################################
#### Compile the server side application
################################

# set the name of the final library
set( MARGOT_AGORA_COMMON_LIB "agora-common" )
set( MARGOT_AGORA_REMOTE_HANDLER "agora-remote-handler" )
set( MARGOT_PSEUDOAPP "margot-pseudoapp" )


# set the header of the application
set( HDR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include" )
set( AGORA_HEADERS
     ${HDR_PATH}/logger.hpp
     ${HDR_PATH}/safe_queue.hpp
     ${HDR_PATH}/remote_handler.hpp
     ${HDR_PATH}/virtual_channel.hpp
     ${HDR_PATH}/threadpool.hpp
     ${HDR_PATH}/fs_handler.hpp
     ${HDR_PATH}/cassandra_fs_implementation.hpp
     ${HDR_PATH}/application_stub.hpp
   )
include_directories( ${HDR_PATH} )


# set the common source files for server-side stuff in a library
set( SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src" )
set( AGORA_SOURCES
     ${SRC_PATH}/logger
     ${SRC_PATH}/paho_remote_implementation
     ${SRC_PATH}/cassandra_fs_implementation
   )
add_library( ${MARGOT_AGORA_COMMON_LIB} STATIC ${AGORA_HEADERS} ${AGORA_SOURCES} )


# compile the executables
add_executable( ${MARGOT_AGORA_REMOTE_HANDLER} ${AGORA_HEADERS} ${SRC_PATH}/main )
add_executable( ${MARGOT_PSEUDOAPP} ${AGORA_HEADERS} ${SRC_PATH}/pseudoapp )


# link the libraries
target_link_libraries( ${MARGOT_AGORA_REMOTE_HANDLER} ${MARGOT_AGORA_COMMON_LIB} ${MQTT_LIBRARIES} ${CASSANDRA_LIBRARIES}  )
target_link_libraries( ${MARGOT_PSEUDOAPP} ${MARGOT_AGORA_COMMON_LIB} ${MQTT_LIBRARIES} ${CASSANDRA_LIBRARIES} )


# add the install command
install( TARGETS ${MARGOT_AGORA_REMOTE_HANDLER} ${MARGOT_PSEUDOAPP} DESTINATION bin )
