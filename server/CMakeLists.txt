################################
#### Compile the server side application
################################

# set the name of the final library
set( MARGOT_SERVER_NAME "margot-agora")
set( MARGOT_APP_STUB "margot-stub" )


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


# set the common source files for server-side stuff
set( SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src" )
set( AGORA_SOURCES
     ${SRC_PATH}/logger
     ${SRC_PATH}/paho_remote_implementation
     ${SRC_PATH}/cassandra_fs_implementation
   )


# add the main file for the server
set( SERVER_SOURCES
     ${AGORA_SOURCES}
     ${SRC_PATH}/main
   )

# add the main file for the stub appication
set( APP_STUB
     ${AGORA_SOURCES}
     ${SRC_PATH}/appstub
   )



# compile the executables
add_executable( ${MARGOT_SERVER_NAME} ${AGORA_HEADERS} ${AGORA_SOURCES} ${SERVER_SOURCES} )
add_executable( ${MARGOT_APP_STUB} ${AGORA_HEADERS} ${AGORA_SOURCES} ${APP_STUB} )


# link the libraries
target_link_libraries( ${MARGOT_SERVER_NAME} ${MQTT_LIBRARIES} ${CASSANDRA_LIBRARIES} )
target_link_libraries( ${MARGOT_APP_STUB} ${MQTT_LIBRARIES} ${CASSANDRA_LIBRARIES} )


# add the install command
install( TARGETS ${MARGOT_SERVER_NAME} ${MARGOT_APP_STUB} DESTINATION bin )
