################################
#### Framework composition
################################


# configure the framework
add_subdirectory( config )

# building the monitor
add_subdirectory( monitor )

# building the asrtm
add_subdirectory( asrtm )





################################
#### Generation of the test
################################



# check if we have found cxxtest
if ( CXXTEST_FOUND )



  # define the name of the executable
  set( MARGOT_TEST_EXE_NAME "margot_test" )

  # get all the test suites from the framework
  set( MARGOT_TEST_SUITE ${MONITOR_TEST_HEADERS} ${ASRTM_TEST_HEADERS})


  # generate the test runner cpp file
  set( TEST_MAIN_FILE ${CMAKE_CURRENT_BINARY_DIR}/margot_test_runner.cpp )
  add_custom_command( OUTPUT ${TEST_MAIN_FILE}
                      COMMAND ${CXXTEST_TESTGEN_EXECUTABLE} --error-printer -o ${TEST_MAIN_FILE} ${MARGOT_TEST_SUITE}
                      DEPENDS ${MARGOT_TEST_SUITE}
                    )


  # set the include dirs for the framework
  include_directories( ${CXXTEST_INCLUDE_DIRS} ${MONITOR_HDR_PATH} ${ASRTM_HDR_PATH} ${CONFIGURATION_HDR_PATH})


  # create the test executable
  add_executable( ${MARGOT_TEST_EXE_NAME} ${TEST_MAIN_FILE} ${MARGOT_TEST_SUITE} )

  # link the framework
  target_link_libraries( ${MARGOT_TEST_EXE_NAME} ${MONITOR_LIB_NAME} ${ASRTM_LIB_NAME} )

  # install the executable
  install( TARGETS ${MARGOT_TEST_EXE_NAME} DESTINATION bin )

endif ( CXXTEST_FOUND )
