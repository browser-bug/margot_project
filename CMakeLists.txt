# check the minimum version
cmake_minimum_required( VERSION 3.4.3 ) # for the imported targets

# the project name
project( mARGOt VERSION 3.0 LANGUAGES CXX)

#########################################################################
#### Change the default  options
#########################################################################

# set the rpath for the generated libraries
include(GNUInstallDirs)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

#########################################################################
#### Compile each component of the framework
#########################################################################

# add the two main component of the margot framework
add_subdirectory( margot )
add_subdirectory( heel )
add_subdirectory( agora )

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
message ( STATUS "//===------------------------===//" )
message ( STATUS "" )
