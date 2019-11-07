# check the minimum version
cmake_minimum_required( VERSION 3.4.3 ) # for the imported targets

# the project name
project( mARGOt VERSION 3.0 LANGUAGES CXX)

# define the option of the whole build
option( WITH_AGORA "Build the remote application handler" OFF)


#########################################################################
#### Change the default  options
#########################################################################

# set the install path chosen by the user
set(MARGOT_DEFAULT_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/install")
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
	message( STATUS "Setting the install path to '${MARGOT_DEFAULT_INSTALL_PREFIX}' as none was specified ")
    set ( CMAKE_INSTALL_PREFIX "${MARGOT_DEFAULT_INSTALL_PREFIX}" CACHE PATH "default install path" FORCE )
endif()
file(WRITE ${CMAKE_SOURCE_DIR}/etc/path.env "INSTALL_PATH=${CMAKE_INSTALL_PREFIX}")

# set the default build type if not specified explicitely by the user
set(MARGOT_DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${MARGOT_DEFAULT_BUILD_TYPE}' as none was specified")
  set(CMAKE_BUILD_TYPE "${MARGOT_DEFAULT_BUILD_TYPE}" CACHE STRING "The type of build" FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif()

# set the rpath for the generated libraries
include(GNUInstallDirs)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

#########################################################################
#### Compile each component of the framework
#########################################################################

# add the two main component of the margot framework
add_subdirectory( margot )
add_subdirectory( heel )

# check if we need to compile also agora
if (WITH_AGORA)
	add_subdirectory( agora )
endif (WITH_AGORA)


#########################################################################
#### Write a summary with the build information
#########################################################################

message ( STATUS "" )
message ( STATUS "//===------------------------===//" )
message ( STATUS "//     MARGOT CONFIG STATUS     //" )
message ( STATUS "//===------------------------===//" )
message ( STATUS "Build directory..................: ${PROJECT_BINARY_DIR}" )
message ( STATUS "Install prefix...................: ${CMAKE_INSTALL_PREFIX}" )
message ( STATUS "Build type.......................: ${CMAKE_BUILD_TYPE}")
message ( STATUS "With remote application handler..: ${WITH_AGORA} (CMake option WITH_AGORA)" )
message ( STATUS "//===------------------------===//" )
message ( STATUS "" )
