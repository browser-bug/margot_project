################################
#### Application general section
################################

# check the minimum version
cmake_minimum_required( VERSION 2.8 )



################################
#### Dependencies check
################################


# Load additional CMake modules
list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")


# check for the papi framework
find_package(MARGOT)
if (NOT HAVE_MARGOT)
  message(FATAL_ERROR "Unable to compile the application without the mARGOt framework")
endif (NOT HAVE_MARGOT)


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
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g3 -O0")

# setting release with debug info flags
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -march=native -g3 -O2 -flto")

# setting release flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -Ofast -funroll-loops -fprefetch-loop-arrays -flto")


# include the autotuner directories
include_directories(${MARGOT_INCLUDES})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})



################################
#### Build section
################################

# define the name of the executable
set( EXE_NAME "margot_benchmark")

# set the source file of the application
set( APP_SRC ${CMAKE_CURRENT_SOURCE_DIR}/main.cc
   )


# set the source file of the application
set( APP_HDR ${CMAKE_CURRENT_SOURCE_DIR}/file_logger.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/bench_base.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/benchmark_knowledge_base.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/benchmark_state.hpp
             ${CMAKE_CURRENT_SOURCE_DIR}/benchmark_asrtm.hpp
  )

# add the target
add_executable( ${EXE_NAME} ${APP_SRC} ${APP_HDR} )

# link the autotuner
target_link_libraries( ${EXE_NAME} ${MARGOT_LIBRARIES} )

# install command
install(TARGETS ${EXE_NAME} DESTINATION bin )
