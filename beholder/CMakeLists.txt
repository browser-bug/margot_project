################################
#### Compile the server side application
################################

# set the name of the final library
set( MARGOT_BEHOLDER_REMOTE_HANDLER "beholder" )

################################
### External Libraries
################################

# Boost program options
find_package(Boost 1.45.0 REQUIRED program_options)


# include the directories
include_directories(${Boost_INCLUDES})


# compile the executables
set( SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src" )
add_executable( ${MARGOT_BEHOLDER_REMOTE_HANDLER} ${SRC_PATH}/main )

# link the libraries
target_link_libraries( ${MARGOT_BEHOLDER_REMOTE_HANDLER} ${MARGOT_LIB_NAME} ${MQTT_LIBRARIES} ${CASSANDRA_LIBRARIES} ${Boost_LIBRARIES} pthread )

# add the install command
install( TARGETS ${MARGOT_BEHOLDER_REMOTE_HANDLER} DESTINATION bin )
