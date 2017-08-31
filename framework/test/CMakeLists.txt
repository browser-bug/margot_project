# the test application name
set( MARGOT_TEST_EXE_NAME "margot_test" )


# the actual tests for the margot framework
set( TEST_HEADER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/test_cases" )
set( TEST_CASE_FILES
	${TEST_HEADER_PATH}/basic_objects.hpp
	${TEST_HEADER_PATH}/operating_point.hpp
	${TEST_HEADER_PATH}/knowledge_base.hpp
	${TEST_HEADER_PATH}/monitor.hpp
	${TEST_HEADER_PATH}/field_adaptor.hpp
	${TEST_HEADER_PATH}/knowledge_adaptor.hpp
	${TEST_HEADER_PATH}/evaluator.hpp
	${TEST_HEADER_PATH}/view.hpp
	${TEST_HEADER_PATH}/rank.hpp
	${TEST_HEADER_PATH}/goal.hpp
	${TEST_HEADER_PATH}/constraint.hpp
	)

# include the required directories
include_directories( ${CXXTEST_INCLUDES} )
include_directories( ${TEST_HEADER_PATH} )


# generate the test main file
set( TEST_MAIN_FILE ${CMAKE_CURRENT_BINARY_DIR}/margot_test_runner.cpp )
add_custom_command(
	OUTPUT ${TEST_MAIN_FILE}
	COMMAND ${CXXTEST_BIN} --error-printer -o ${TEST_MAIN_FILE} ${TEST_CASE_FILES}
	DEPENDS ${TEST_CASE_FILES}
	)

# add the target to build the test case
add_executable( ${MARGOT_TEST_EXE_NAME} ${TEST_MAIN_FILE} ${TEST_CASE_FILES} )


# install the test binary
install( TARGETS ${MARGOT_TEST_EXE_NAME} DESTINATION bin )
