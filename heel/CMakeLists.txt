# check the minimum version
cmake_minimum_required( VERSION 3.4.3 ) # for the imported targets

# the project name
project( mARGOt VERSION 3.0 LANGUAGES CXX)

#########################################################################
#### Change the default  options
#########################################################################

# this is a common rule
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set ( CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}" CACHE PATH "default install path" FORCE )
endif()

# set the default build type if not specified explicitely by the user
set(HELL_DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${HELL_DEFAULT_BUILD_TYPE}' as none was specified")
  set(CMAKE_BUILD_TYPE "${HELL_DEFAULT_BUILD_TYPE}" CACHE STRING "The type of build" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif()


#########################################################################
#### Look for required external libraries, according to the options
#########################################################################


#########################################################################
#### Define the mARGOt heel library target
#########################################################################





#########################################################################
#### Define the mARGOt heel executable
#########################################################################

# define the target name and paths
set( HEEL_BINARY_NAME "margot_heel")
set( HEEL_HDR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include" )
set( HEEL_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src")

# define the related headers and source files
set( HEEL_HEADERS
	)
set( HEEL_SOURCES
	${HEEL_SRC_PATH}/main
)

# declare the target
add_executable( ${HEEL_BINARY_NAME} ${HEEL_SOURCES} ${HEEL_HEADERS} )

# define its interface
target_include_directories( ${HEEL_BINARY_NAME} PRIVATE ${HEEL_HDR_PATH} )


# define its compiler options
target_compile_features( ${HEEL_BINARY_NAME} PRIVATE cxx_std_17 ) # for the filesystem component
target_compile_options( ${HEEL_BINARY_NAME} PRIVATE "-Wall" )
target_compile_options( ${HEEL_BINARY_NAME} PRIVATE $<$<CONFIG:DEBUG>: -g -O0 >)
target_compile_options( ${HEEL_BINARY_NAME} PRIVATE $<$<CONFIG:RELWITHDEBINFO>: -march=native -mtune=native -g -O2 >)
target_compile_options( ${HEEL_BINARY_NAME} PRIVATE $<$<CONFIG:RELEASE>: -march=native -mtune=native -O3 >)




#########################################################################
#### Define the install settings for all the stuff
#########################################################################

include(GNUInstallDirs)
install(TARGETS ${HEEL_BINARY_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )