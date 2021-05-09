# check the minimum version
cmake_minimum_required( VERSION 3.14 )

# the project name
project( AGORA VERSION 2.0 LANGUAGES CXX)

#########################################################################
#### Look for required external libraries
#########################################################################
find_package(Boost REQUIRED COMPONENTS program_options)

# include standard gnu path (to make sure to have them when needed)
include(GNUInstallDirs)

#########################################################################
#### Define the Agora server executable
#########################################################################

# define the target name
set( AGORA_BINARY_NAME "agora")

set( AGORA_BINARY_HDR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include" )
set( AGORA_BINARY_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src")

# define the related headers and source files
set( AGORA_BINARY_HEADERS
	)
set( AGORA_BINARY_SOURCES
  ${AGORA_BINARY_SRC_PATH}/main
)

# declare the target
add_executable( ${AGORA_BINARY_NAME} ${AGORA_BINARY_SOURCES} ${AGORA_BINARY_HEADERS} )
add_executable( margot::${AGORA_BINARY_NAME} ALIAS ${AGORA_BINARY_NAME} )

# set the executable dependencies
target_link_libraries( ${AGORA_BINARY_NAME} PUBLIC margot::agora_common Boost::program_options)

# set the runtime library path for executing it anywhere
set_target_properties(${AGORA_BINARY_NAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)


# define its compiler options
target_compile_features( ${AGORA_BINARY_NAME} PRIVATE cxx_std_17 ) # for the filesystem component
include(CheckIPOSupported)
check_ipo_supported(RESULT result)
if(result)
  set_target_properties( ${AGORA_BINARY_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE )
endif()

#########################################################################
#### Define the Agora client executable (for TESTING purposes)
#########################################################################
set( AGORA_CLIENT_BINARY_NAME "client")

# define the related headers and source files
set( AGORA_CLIENT_BINARY_HEADERS
	)
set( AGORA_CLIENT_BINARY_SOURCES
	${AGORA_BINARY_SRC_PATH}/client
)

# declare the target
add_executable( ${AGORA_CLIENT_BINARY_NAME} EXCLUDE_FROM_ALL ${AGORA_CLIENT_BINARY_SOURCES} ${AGORA_CLIENT_BINARY_HEADERS} )
add_executable( margot::${AGORA_CLIENT_BINARY_NAME} ALIAS ${AGORA_CLIENT_BINARY_NAME} )

# set the executable dependencies
target_link_libraries( ${AGORA_CLIENT_BINARY_NAME} PUBLIC margot::agora_common Boost::program_options)

# set the runtime library path for executing it anywhere
set_target_properties(${AGORA_CLIENT_BINARY_NAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)


# define its compiler options
target_compile_features( ${AGORA_CLIENT_BINARY_NAME} PRIVATE cxx_std_17 ) # for the filesystem component
include(CheckIPOSupported)
check_ipo_supported(RESULT result)
if(result)
  set_target_properties( ${AGORA_CLIENT_BINARY_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE )
endif()

#########################################################################
#### Define the install settings for all the stuff
#########################################################################

# install the agora application binary
install(TARGETS ${AGORA_BINARY_NAME}
    EXPORT  ${AGORA_BINARY_NAME}Targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# generate the config files to find the executable
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${AGORA_BINARY_NAME}ConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY ExactVersion
)
install(EXPORT ${AGORA_BINARY_NAME}Targets
  FILE ${AGORA_BINARY_NAME}Targets.cmake
    NAMESPACE margot::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/margot
)
