################################
#### Compile the server side application
################################

# set the name of the final library
set( MARGOT_SERVER_NAME "margot-agora")


# set the header of the application
set( HDR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include" )
set( AGORA_HEADERS
     ${HDR_PATH}/logger.hpp
     ${HDR_PATH}/safe_queue.hpp
     ${HDR_PATH}/remote_handler.hpp
     ${HDR_PATH}/virtual_channel.hpp
     ${HDR_PATH}/threadpool.hpp
   )
include_directories( ${HDR_PATH} )


# set the source files for the application
set( SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src" )
set( AGORA_SOURCES
     ${SRC_PATH}/main
     ${SRC_PATH}/logger
     ${SRC_PATH}/paho_remote_implementation
   )



# compile the executable
add_executable( ${MARGOT_SERVER_NAME} ${AGORA_HEADERS} ${AGORA_SOURCES} )

# link the libraries
target_link_libraries( ${MARGOT_SERVER_NAME} ${MARGOT_LIB_NAME} )
target_link_libraries( ${MARGOT_SERVER_NAME} ${MQTT_LIBRARIES} )


# add the install command
install( TARGETS ${MARGOT_SERVER_NAME} DESTINATION bin )
