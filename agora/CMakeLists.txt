################################
#### Compile the server side application
################################

# set the name of the final library
set( MARGOT_AGORA_REMOTE_HANDLER "agora" )

# compile the executables
set( SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src" )
add_executable( ${MARGOT_AGORA_REMOTE_HANDLER} ${SRC_PATH}/main )

# link the libraries
target_link_libraries( ${MARGOT_AGORA_REMOTE_HANDLER} ${MARGOT_LIB_NAME} ${MQTT_LIBRARIES} ${CASSANDRA_LIBRARIES}  )

# add the install command
install( TARGETS ${MARGOT_AGORA_REMOTE_HANDLER} DESTINATION bin )
