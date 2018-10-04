################################
#### HEEL general section
################################

# check the minimum version
cmake_minimum_required( VERSION 2.8 )

# the project name
project( MARGOT_HEEL_IF )

# The version number.
set( PROJECT_VERSION_MAJOR 1 )
set( PROJECT_VERSION_MINOR 0 )



################################
#### Option section
################################

message ( STATUS "" )
message ( STATUS "*********************************" )
message ( STATUS "**        CONFIG STATUS        **" )
message ( STATUS "*********************************" )
message ( STATUS "Install prefix..................................: ${CMAKE_INSTALL_PREFIX}" )

# tell MARGOT interface to log information in a "unique" file per run
option( LOG_USE_PID "The margot log file name includes the PID" OFF )
if ( LOG_USE_PID )
	set( MARGOT_EXTRA_CXXFLAGS "${MARGOT_EXTRA_CXXFLAGS} -DMARGOT_LOG_USE_PID_IDENTIFIER" )
	message ( STATUS "Margot uniques log file names...................: ENABLED!    (-DLOG_USE_PID=OFF to disable)" )
else ( LOG_USE_PID )
	set( MARGOT_EXTRA_CXXFLAGS "${MARGOT_EXTRA_CXXFLAGS}" )
	message ( STATUS "Margot uniques log file names...................: DISABLED!   (-DLOG_USE_PID=ON to enable)" )
endif ( LOG_USE_PID )

# tell MARGOT interface to disable the log on the standard output
option( WITH_LOG_STDOUT "Enable the log on the standard output" ON )
if ( WITH_LOG_STDOUT )
	message ( STATUS "Log on the standard output......................: ENABLED!    (-WITH_LOG_STDOUT=OFF to disable)" )
else ( WITH_LOG_STDOUT )
	set( MARGOT_EXTRA_CXXFLAGS "${MARGOT_EXTRA_CXXFLAGS} -DMARGOT_DISABLE_LOG_STDOUT" )
	message ( STATUS "Log on the standard output......................: DISABLED!   (-WITH_LOG_STDOUT=ON to enable)" )
endif ( WITH_LOG_STDOUT )

# tell MARGOT interface to disable the log on the standard output
option( WITH_LOG_FILE "Enable the log on file" ON )
if ( WITH_LOG_FILE )
	message ( STATUS "Log on file.....................................: ENABLED!    (-WITH_LOG_FILE=OFF to disable)" )
else ( WITH_LOG_FILE )
	set( MARGOT_EXTRA_CXXFLAGS "${MARGOT_EXTRA_CXXFLAGS} -DMARGOT_DISABLE_LOG_FILE" )
	message ( STATUS "Log on file.....................................: DISABLED!   (-WITH_LOG_FILE=ON to enable)" )
endif ( WITH_LOG_FILE )

# tell the MARGOT file from the op_list ones
file(GLOB CONF_FILES_FOUND RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/config config/*.conf)
list(SORT CONF_FILES_FOUND)
list(GET CONF_FILES_FOUND 0 FIRST_CONF_FILE)
set( MARGOT_CONF_FILE "${FIRST_CONF_FILE}" CACHE STRING "The configuration file used to generate the high-level interface" )
set_property( CACHE MARGOT_CONF_FILE PROPERTY STRINGS ${CONF_FILES_FOUND})
message(STATUS "MARGOT configuration file ......................: ${MARGOT_CONF_FILE}" )
message(STATUS "     NOTE: the following files are assumed to be Operating Points lists:")
list(REMOVE_ITEM CONF_FILES_FOUND ${MARGOT_CONF_FILE})
set( OP_LIST_FILES_FOUND "")
foreach( CONFIG_FILE IN LISTS CONF_FILES_FOUND )
	message(STATUS "                  - ${CONFIG_FILE}")
	list(APPEND OP_LIST_FILES_FOUND "${CMAKE_CURRENT_SOURCE_DIR}/config/${CONFIG_FILE}" )
endforeach( CONFIG_FILE )



################################
#### Dependencies check
################################

# Load additional CMake modules
list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# check for the MARGOT framework
find_package(MARGOT)

# include the right directories
include_directories( ${MARGOT_INCLUDES} )




################################
#### Compile flags
################################

# force the Release build if not already setted
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif(NOT CMAKE_BUILD_TYPE)

# setting common c++ flags
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MARGOT_EXTRA_CXXFLAGS} -std=c++0x" )

# setting debug flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g -O0 -fdiagnostics-color=always")

# setting release with debug info flags
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -march=native -g3 -O2")

# setting release flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -Ofast -funroll-loops -fprefetch-loop-arrays")




################################
#### Generate the source files
################################

message(STATUS "Generating the glue code")
execute_process( COMMAND "python"  "${MARGOT_CLI_COMMAND}" "generate" "--margot" "${CMAKE_CURRENT_SOURCE_DIR}/config/${MARGOT_CONF_FILE}" "--out" "${CMAKE_CURRENT_SOURCE_DIR}" ${OP_LIST_FILES_FOUND}
                 RESULT_VARIABLE GENERATE_RESULT
)
if(NOT ${GENERATE_RESULT} EQUAL "0")
	message(FATAL_ERROR "Unable to generate the glue code")
endif(NOT ${GENERATE_RESULT} EQUAL "0")



################################
#### Build the library
################################

set( MARGOT_HEEL_LIB_NAME "margot_heel")

# get the list of all the source files
file(GLOB GLUE_CODE_SOURCE_FILES "*.cc")
file(GLOB C_HEADERS "*.h")
file(GLOB CPP_HEADERS "*.hpp")

# set the include_directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${MARGOT_INCLUDES})

# compile the library
add_library( ${MARGOT_HEEL_LIB_NAME} STATIC ${GLUE_CODE_SOURCE_FILES} )

# link the library
target_link_libraries( ${MARGOT_HEEL_LIB_NAME} ${MARGOT_LIBRARIES} )





################################
#### Configure this module
################################


# forward the dependencies from margot
set( MARGOT_DEPS_HEADERS ${MARGOT_INCLUDES} )
set( MARGOT_DEPS_LIBS ${MARGOT_LIBRARIES} )
string (REPLACE ";" " " MARGOT_DEPS_LIBS_STR "${MARGOT_DEPS_LIBS}")
string (REPLACE ";" " -I" MARGOT_DEPS_HEADERS_STR "${MARGOT_DEPS_HEADERS}")




# customizing the pkg-config file
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/margot_heel.pc.in"
  "${CMAKE_CURRENT_BINARY_DIR}/margot_heel.pc"
  @ONLY
  )



# ccustomizing the FindHEEL.cmake
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/FindMARGOT_HEEL.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/FindMARGOT_HEEL.cmake"
  @ONLY
  )
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/FindMARGOT_HEEL.cmake.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindMARGOT_HEEL.cmake"
  @ONLY
  )




################################
#### Install the components
################################
install(TARGETS ${MARGOT_HEEL_LIB_NAME} DESTINATION lib )
install(FILES ${C_HEADERS} DESTINATION include )
install(FILES ${CPP_HEADERS} DESTINATION include )
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/margot_heel.pc" DESTINATION lib/pkgconfig )
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/FindMARGOT_HEEL.cmake" DESTINATION lib/cmake )
