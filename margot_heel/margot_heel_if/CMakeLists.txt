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
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x" )

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
execute_process( COMMAND "python"  "${MARGOT_CLI_COMMAND}/margot_cli" "generate" "--margot" "${CMAKE_CURRENT_SOURCE_DIR}/config/${MARGOT_CONF_FILE}" "--out" "${CMAKE_CURRENT_SOURCE_DIR}" ${OP_LIST_FILES_FOUND}
                 RESULT_VARIABLE GENERATE_RESULT
)
if(NOT ${GENERATE_RESULT} EQUAL "0")
	message(FATAL_ERROR "Unable to generate the glue code")
endif(NOT ${GENERATE_RESULT} EQUAL "0")






################################
#### Build the library
################################

set( LIB_NAME "margot_heel_if")

# get the list of all the source files
file(GLOB GLUE_CODE_SOURCE_FILES "*.cc")
file(GLOB C_HEADERS "*.h")
file(GLOB CPP_HEADERS "*.hpp")

# set the include_directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${MARGOT_INCLUDES})

# compile the library
add_library( ${LIB_NAME} STATIC ${GLUE_CODE_SOURCE_FILES} )

# link the library
target_link_libraries( ${LIB_NAME} ${MARGOT_LIBRARIES} )





################################
#### Configure this module
################################

# customizing the pkg-config file
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/heel.pc.in"
  "${CMAKE_CURRENT_BINARY_DIR}/heel.pc"
  @ONLY
  )



# ccustomizing the FindHEEL.cmake
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/FindHEEL.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/FindHEEL.cmake"
  @ONLY
  )
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/FindHEEL.cmake.in"
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/FindHEEL.cmake"
  @ONLY
  )




################################
#### Install the components
################################
install(TARGETS ${LIB_NAME} DESTINATION lib )
install(FILES ${C_HEADERS} DESTINATION include )
install(FILES ${CPP_HEADERS} DESTINATION include )
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/heel.pc" DESTINATION lib/pkgconfig )
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/FindHEEL.cmake" DESTINATION lib/cmake )
