# check the minimum version
cmake_minimum_required( VERSION 3.14 )

# the project name
project( AGORA VERSION 2.0 LANGUAGES CXX)

#########################################################################
#### Look for required external libraries
#########################################################################

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads REQUIRED)

#find_package(eclipse-paho-mqtt-c REQUIRED)

find_package(Boost REQUIRED COMPONENTS program_options)

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
target_compile_options( ${AGORA_BINARY_NAME} PRIVATE "-Wall" )
target_compile_options( ${AGORA_BINARY_NAME} PRIVATE $<$<CONFIG:DEBUG>: -g -O0 >)
target_compile_options( ${AGORA_BINARY_NAME} PRIVATE $<$<CONFIG:RELWITHDEBINFO>: -march=native -mtune=native -g -O2 >)
target_compile_options( ${AGORA_BINARY_NAME} PRIVATE $<$<CONFIG:RELEASE>: -march=native -mtune=native -O3 >)
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
add_executable( ${AGORA_CLIENT_BINARY_NAME} ${AGORA_CLIENT_BINARY_SOURCES} ${AGORA_CLIENT_BINARY_HEADERS} )
add_executable( margot::${AGORA_CLIENT_BINARY_NAME} ALIAS ${AGORA_CLIENT_BINARY_NAME} )

# set the executable dependencies
target_link_libraries( ${AGORA_CLIENT_BINARY_NAME} PUBLIC margot::agora_common Boost::program_options)

# set the runtime library path for executing it anywhere
set_target_properties(${AGORA_CLIENT_BINARY_NAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)


# define its compiler options
target_compile_features( ${AGORA_CLIENT_BINARY_NAME} PRIVATE cxx_std_17 ) # for the filesystem component
target_compile_options( ${AGORA_CLIENT_BINARY_NAME} PRIVATE "-Wall" )
target_compile_options( ${AGORA_CLIENT_BINARY_NAME} PRIVATE $<$<CONFIG:DEBUG>: -g -O0 >)
target_compile_options( ${AGORA_CLIENT_BINARY_NAME} PRIVATE $<$<CONFIG:RELWITHDEBINFO>: -march=native -mtune=native -g -O2 >)
target_compile_options( ${AGORA_CLIENT_BINARY_NAME} PRIVATE $<$<CONFIG:RELEASE>: -march=native -mtune=native -O3 >)
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
