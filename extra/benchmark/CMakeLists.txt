

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
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -march=native -g3 -O2")

# setting release flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -Ofast -funroll-loops -fprefetch-loop-arrays")



################################
#### Build section
################################

# define the name of the executable
set( BENCHMARK_NAME "margot_benchmark")

# set the source file of the application
set( BENCHMARK_SRC ${CMAKE_CURRENT_SOURCE_DIR}/main.cc
   )


# set the source file of the application
set( BENCHMARK_HDR
  ${CMAKE_CURRENT_SOURCE_DIR}/result_printer.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/evaluator.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_add_constraint.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_add_ops.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_select_data_feature.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_set_rank.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_update_scaling.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/test_update.hpp
  )

# add the target
add_executable( ${BENCHMARK_NAME} ${BENCHMARK_SRC} ${BENCHMARK_HDR} )

# link the autotuner
target_link_libraries( ${BENCHMARK_NAME} ${MARGOT_LIB_NAME} )

# install command
install(TARGETS ${BENCHMARK_NAME} DESTINATION bin )
