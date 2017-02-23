################################
#### mARGOt general section
################################

# check the minimum version
cmake_minimum_required( VERSION 2.8 )

# the project name
project( MARGOT )

# The version number.
set( PROJECT_VERSION_MAJOR 2 )
set( PROJECT_VERSION_MINOR 9 )








################################
#### Option section
################################

# obtain the kind of lib
option( LIB_STATIC "Build static library" ON )

# check if we want to use type-safe casting
option( SAFE_CASTING "Use RTTI to perform safe casting" ON )

# check to see if we want pedantic check at runtime
option( PEDANTIC_CHECK "Extra error checks at runtime" ON )

# check if we need to build also the Doxygen documentation
option( WITH_DOC "Build the framework documentation" OFF )

# check if we need to compute the reward
option( ENABLE_REWARD "Enable the reward computation of a configuration" OFF )

# check if we need to log the behaviour of the learning system
option( ENABLE_LEARNING "Enable the learning state (and also the reward computation)" OFF )

# check if we need to log the behaviour of the learning system
option( LOG_LEARNING "Log the behaviour of the learning framework to a file" OFF )







################################
#### Dependencies check
################################

# Load additional CMake modules
list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Check for doxygen
if ( WITH_DOC )
  find_package(Doxygen)
endif ( WITH_DOC )


# Check for cxxtest
find_package(CxxTest)


# check for the papi framework
find_package(PAPI)



# check for the sensor library
find_package(SENSORS)


# check for the collector library
find_package(COLLECTOR)







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
#### Configuration section
################################

# ---- Name the framework components
set( MONITOR_LIB_NAME "margot_monitor" )
set( ASRTM_LIB_NAME "margot_asrtm" )






# ---- Set the include directory for the project headers
set( MONITOR_HDR_PATH ${CMAKE_CURRENT_SOURCE_DIR}/framework/monitor/include )
set( ASRTM_HDR_PATH ${CMAKE_CURRENT_SOURCE_DIR}/framework/asrtm/include )
set( CONFIGURATION_HDR_PATH ${CMAKE_CURRENT_BINARY_DIR}/framework/include )
set( CONFIG_FILE ${CMAKE_CURRENT_BINARY_DIR}/framework/include/margot/config.hpp )





################################
#### Build section
################################



# building the framework
add_subdirectory( framework )



################################
#### Install the margot cli
################################

install( DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/margot_heel/margot_heel_cli DESTINATION bin )
install( FILES ${CMAKE_CURRENT_SOURCE_DIR}/margot_heel/margot_heel_cli/bin/margot_cli DESTINATION bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
