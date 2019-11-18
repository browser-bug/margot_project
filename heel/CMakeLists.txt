# check the minimum version
cmake_minimum_required( VERSION 3.9 ) # for the imported targets and lto

# the project name
project( HEEL VERSION 3.0 LANGUAGES CXX)

#########################################################################
#### Look for required external libraries
#########################################################################

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED COMPONENTS program_options)


#########################################################################
#### Define the mARGOt heel library target
#########################################################################

# define the target name and paths
set( HEEL_LIBRARY_NAME "margot_heel")
set( HEEL_HDR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include" )
set( HEEL_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src")

# define the related headers and source files
set( HEEL_LIBRARY_HEADERS
	"${HEEL_HDR_PATH}/heel/generator_description_verbose.hpp"
	"${HEEL_HDR_PATH}/heel/generator_cpp_application_geometry_hdr.hpp"
	"${HEEL_HDR_PATH}/heel/generator_cpp_application_geometry_src.hpp"
	"${HEEL_HDR_PATH}/heel/generator_cpp_knowledge_hdr.hpp"
	"${HEEL_HDR_PATH}/heel/generator_cpp_knowledge_src.hpp"
	"${HEEL_HDR_PATH}/heel/generator_cpp_managers_hdr.hpp"
	"${HEEL_HDR_PATH}/heel/generator_cpp_managers_src.hpp"
	"${HEEL_HDR_PATH}/heel/generator_source_file.hpp"
	"${HEEL_HDR_PATH}/heel/generator_utils.hpp"
	"${HEEL_HDR_PATH}/heel/model_agora.hpp"
	"${HEEL_HDR_PATH}/heel/model_application.hpp"
	"${HEEL_HDR_PATH}/heel/model_block.hpp"
	"${HEEL_HDR_PATH}/heel/model_features.hpp"
	"${HEEL_HDR_PATH}/heel/model_knob.hpp"
	"${HEEL_HDR_PATH}/heel/model_metric.hpp"
	"${HEEL_HDR_PATH}/heel/model_monitor.hpp"
	"${HEEL_HDR_PATH}/heel/model_operating_point.hpp"
	"${HEEL_HDR_PATH}/heel/model_parameter.hpp"
	"${HEEL_HDR_PATH}/heel/model_state.hpp"
	"${HEEL_HDR_PATH}/heel/model_validate.hpp"
	"${HEEL_HDR_PATH}/heel/parser_agora.hpp"
	"${HEEL_HDR_PATH}/heel/parser_application.hpp"
	"${HEEL_HDR_PATH}/heel/parser_block.hpp"
	"${HEEL_HDR_PATH}/heel/parser_features.hpp"
	"${HEEL_HDR_PATH}/heel/parser_knob.hpp"
	"${HEEL_HDR_PATH}/heel/parser_metric.hpp"
	"${HEEL_HDR_PATH}/heel/parser_monitor.hpp"
	"${HEEL_HDR_PATH}/heel/parser_operating_point.hpp"
	"${HEEL_HDR_PATH}/heel/parser_parameter.hpp"
	"${HEEL_HDR_PATH}/heel/parser_state.hpp"
	"${HEEL_HDR_PATH}/heel/parser_utils.hpp"
	"${HEEL_HDR_PATH}/heel/configuration_file.hpp"
	"${HEEL_HDR_PATH}/heel/cpp_enum_conversion.hpp"
	"${HEEL_HDR_PATH}/heel/cpp_parser_gen.hpp"
	"${HEEL_HDR_PATH}/heel/cpp_state_emitter.hpp"
	"${HEEL_HDR_PATH}/heel/cpp_utils.hpp"
	"${HEEL_HDR_PATH}/heel/json_parser.hpp"
	"${HEEL_HDR_PATH}/heel/logger.hpp"
	"${HEEL_HDR_PATH}/heel/print_application_model.hpp"
	"${HEEL_HDR_PATH}/heel/typer.hpp"
	"${HEEL_HDR_PATH}/heel/workspace.hpp"
)
set( HEEL_LIBRARY_SOURCES
	"${HEEL_SRC_PATH}/configuration_file.cpp"
	"${HEEL_SRC_PATH}/cpp_state_emitter.cpp"
	"${HEEL_SRC_PATH}/generator_description_verbose.cpp"
	"${HEEL_SRC_PATH}/generator_cpp_application_geometry_hdr.cpp"
	"${HEEL_SRC_PATH}/generator_cpp_application_geometry_src.cpp"
	"${HEEL_SRC_PATH}/generator_cpp_knowledge_hdr.cpp"
	"${HEEL_SRC_PATH}/generator_cpp_knowledge_src.cpp"
	"${HEEL_SRC_PATH}/generator_cpp_managers_hdr.cpp"
	"${HEEL_SRC_PATH}/generator_cpp_managers_src.cpp"
	"${HEEL_SRC_PATH}/generator_utils.cpp"
	"${HEEL_SRC_PATH}/model_agora.cpp"
	"${HEEL_SRC_PATH}/model_features.cpp"
	"${HEEL_SRC_PATH}/model_knob.cpp"
	"${HEEL_SRC_PATH}/model_metric.cpp"
	"${HEEL_SRC_PATH}/model_monitor.cpp"
	"${HEEL_SRC_PATH}/model_state.cpp"
	"${HEEL_SRC_PATH}/model_validate.cpp"
	"${HEEL_SRC_PATH}/parser_agora.cpp"
	"${HEEL_SRC_PATH}/parser_application.cpp"
	"${HEEL_SRC_PATH}/parser_block.cpp"
	"${HEEL_SRC_PATH}/parser_features.cpp"
	"${HEEL_SRC_PATH}/parser_knob.cpp"
	"${HEEL_SRC_PATH}/parser_metric.cpp"
	"${HEEL_SRC_PATH}/parser_monitor.cpp"
	"${HEEL_SRC_PATH}/parser_operating_point.cpp"
	"${HEEL_SRC_PATH}/parser_parameter.cpp"
	"${HEEL_SRC_PATH}/parser_state.cpp"
	"${HEEL_SRC_PATH}/parser_utils.cpp"
	"${HEEL_SRC_PATH}/typer.cpp"
	"${HEEL_SRC_PATH}/workspace.cpp"
)

# declare the target
add_library( ${HEEL_LIBRARY_NAME} SHARED ${HEEL_LIBRARY_SOURCES} ${HEEL_LIBRARY_HEADERS} )
add_library( margot::${HEEL_LIBRARY_NAME} ALIAS ${HEEL_LIBRARY_NAME} )

# define its interface
target_include_directories( ${HEEL_LIBRARY_NAME} PRIVATE ${HEEL_HDR_PATH} )
set_target_properties( ${HEEL_LIBRARY_NAME} PROPERTIES PUBLIC_HEADER "${HEEL_LIBRARY_HEADERS}" )

# set the executable dependencies
target_link_libraries( ${HEEL_LIBRARY_NAME} PUBLIC Boost::boost Boost::program_options )

# define its compiler options
target_compile_features( ${HEEL_LIBRARY_NAME} PRIVATE cxx_std_17 ) # for the filesystem component
target_compile_options( ${HEEL_LIBRARY_NAME} PRIVATE "-Wall" )
target_compile_options( ${HEEL_LIBRARY_NAME} PRIVATE $<$<CONFIG:DEBUG>: -g -O0 >)
target_compile_options( ${HEEL_LIBRARY_NAME} PRIVATE $<$<CONFIG:RELWITHDEBINFO>: -march=native -mtune=native -g -O2 >)
target_compile_options( ${HEEL_LIBRARY_NAME} PRIVATE $<$<CONFIG:RELEASE>: -march=native -mtune=native -O3 >)



#########################################################################
#### Define the mARGOt heel executable
#########################################################################

# define the target name
set( HEEL_BINARY_NAME "margot_heel_generator")

# define the related headers and source files
set( HEEL_BINARY_HEADERS
	)
set( HEEL_BINARY_SOURCES
	${HEEL_SRC_PATH}/main
)

# declare the target
add_executable( ${HEEL_BINARY_NAME} ${HEEL_BINARY_SOURCES} ${HEEL_BINARY_HEADERS} )
add_executable( margot::${HEEL_BINARY_NAME} ALIAS ${HEEL_BINARY_NAME} )

# define its interface
target_include_directories( ${HEEL_BINARY_NAME} PRIVATE ${HEEL_HDR_PATH} )

# set the executable dependencies
target_link_libraries( ${HEEL_BINARY_NAME} PUBLIC ${HEEL_LIBRARY_NAME} )

# set the runtime library path for executing it anywhere
set_target_properties(${HEEL_BINARY_NAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)


# define its compiler options
target_compile_features( ${HEEL_BINARY_NAME} PRIVATE cxx_std_17 ) # for the filesystem component
target_compile_options( ${HEEL_BINARY_NAME} PRIVATE "-Wall" )
target_compile_options( ${HEEL_BINARY_NAME} PRIVATE $<$<CONFIG:DEBUG>: -g -O0 >)
target_compile_options( ${HEEL_BINARY_NAME} PRIVATE $<$<CONFIG:RELWITHDEBINFO>: -march=native -mtune=native -g -O2 >)
target_compile_options( ${HEEL_BINARY_NAME} PRIVATE $<$<CONFIG:RELEASE>: -march=native -mtune=native -O3 >)
include(CheckIPOSupported)
check_ipo_supported(RESULT result)
if(result)
  set_target_properties( ${HEEL_BINARY_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE )
endif()

# Define the CMake function that generates the high-level interface for
# the application, to be used when compiling the application
# The expected parameter is the margot configuration file. If the user
# would like to provide the application knowledge as a list of Operating
# Points, he/she can append them when calling the function
set(DEFAULT_MARGOT_HEEL_INTERFACE_CMAKE_PATH "${CMAKE_CURRENT_BINARY_DIR}/margot_heel_interface.cmake"
	CACHE INTERNAL "")
function(margot_heel_generate_interface MARGOT_CONFIG_FILE )

       # check if we actually have the margot configuration file
       if ("${ARGC}" EQUAL "0")
               message(FATAL_ERROR "Unable to generate the application interface without configuration files")
               message(STATUS "Function usage:")
               message(STATUS "margot_heel_generate_interface( <PATH_MARGOT_CONFIG_FILE> [<PATH_OP_LIST>]*")
       endif ("${ARGC}" EQUAL "0")


       # now we have to create and customize the CMake project for the interface
       include(GNUInstallDirs)
       set(MARGOT_CONFIG_PATH "${MARGOT_CONFIG_FILE}")
       set(MARGOT_OPS_PATH "${ARGN}")
       configure_file("${DEFAULT_MARGOT_HEEL_INTERFACE_CMAKE_PATH}"
                      "${CMAKE_CURRENT_BINARY_DIR}/margot_heel_interface/CMakeLists.txt" COPYONLY)

       # finally, we had that project
       add_subdirectory( ${CMAKE_CURRENT_BINARY_DIR}/margot_heel_interface )

endfunction(margot_heel_generate_interface)



#########################################################################
#### Define the install settings for all the stuff
#########################################################################

# install all the target generated by heel
install(TARGETS ${HEEL_LIBRARY_NAME}
		EXPORT ${HEEL_LIBRARY_NAME}Targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/heel
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/heel
)
install(TARGETS ${HEEL_BINARY_NAME}
		EXPORT  ${HEEL_BINARY_NAME}Targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# generate the config files to find the library and the executable
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${HEEL_LIBRARY_NAME}ConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY ExactVersion
)
write_basic_package_version_file(
    ${HEEL_BINARY_NAME}ConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY ExactVersion
)
install(EXPORT ${HEEL_LIBRARY_NAME}Targets
    FILE ${HEEL_LIBRARY_NAME}Targets.cmake
    NAMESPACE margot::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/margot
)
install(EXPORT ${HEEL_BINARY_NAME}Targets
    FILE ${HEEL_BINARY_NAME}Targets.cmake
    NAMESPACE margot::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/margot
)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/heelConfig.cmake.in ${HEEL_LIBRARY_NAME}Config.cmake @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/heel_generatorConfig.cmake.in
	           ${HEEL_BINARY_NAME}Config.cmake @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${HEEL_LIBRARY_NAME}Config.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/${HEEL_LIBRARY_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/margot
        )
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${HEEL_BINARY_NAME}Config.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/${HEEL_BINARY_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/margot
        )

# configure the cmake file for the high-level interface
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config/margot_heel_interface.cmake.in"
	           "${DEFAULT_MARGOT_HEEL_INTERFACE_CMAKE_PATH}" @ONLY)
install(FILES "${DEFAULT_MARGOT_HEEL_INTERFACE_CMAKE_PATH}"
	    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/margot)