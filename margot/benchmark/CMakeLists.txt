
#########################################################################
#### Define the mARGOt benchmark application target
#########################################################################

# define the name of the executable
set( BENCHMARK_NAME "margot_benchmark")

# set the source file of the application
set( BENCHMARK_SRC ${CMAKE_CURRENT_SOURCE_DIR}/main.cc )


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

# define its interface
target_include_directories( ${BENCHMARK_NAME} PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# define the linker options
target_link_libraries( ${BENCHMARK_NAME} PRIVATE ${MARGOT_LIBRARY_NAME} )

# install the test binary
install(TARGETS ${BENCHMARK_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} )



#########################################################################
#### Define the compiler options for this target
#########################################################################

target_compile_features( ${MARGOT_LIBRARY_NAME} PRIVATE cxx_std_11 )
target_compile_options(${MARGOT_LIBRARY_NAME} PRIVATE -march=native -mtune=native -O3)




