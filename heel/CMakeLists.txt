# check the minimum version
cmake_minimum_required( VERSION 3.9 ) # for the imported targets and lto

# the project name
project( HEEL VERSION 3.0 LANGUAGES CXX)

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
#### Look for required external libraries
#########################################################################

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED)


#########################################################################
#### Define the mARGOt heel library target
#########################################################################

# define the target name and paths
set( HEEL_LIBRARY_NAME "heel")
set( HEEL_HDR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include" )
set( HEEL_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src")

# define the related headers and source files
set( HEEL_LIBRARY_HEADERS
	"${HEEL_HDR_PATH}/heel/generator/description_verbose.hpp"
	"${HEEL_HDR_PATH}/heel/generator/utils.hpp"
	"${HEEL_HDR_PATH}/heel/model/agora.hpp"
	"${HEEL_HDR_PATH}/heel/model/application.hpp"
	"${HEEL_HDR_PATH}/heel/model/block.hpp"
	"${HEEL_HDR_PATH}/heel/model/features.hpp"
	"${HEEL_HDR_PATH}/heel/model/knob.hpp"
	"${HEEL_HDR_PATH}/heel/model/metric.hpp"
	"${HEEL_HDR_PATH}/heel/model/monitor.hpp"
	"${HEEL_HDR_PATH}/heel/model/parameter.hpp"
	"${HEEL_HDR_PATH}/heel/parser/agora.hpp"
	"${HEEL_HDR_PATH}/heel/parser/application.hpp"
	"${HEEL_HDR_PATH}/heel/parser/block.hpp"
	"${HEEL_HDR_PATH}/heel/parser/features.hpp"
	"${HEEL_HDR_PATH}/heel/parser/knob.hpp"
	"${HEEL_HDR_PATH}/heel/parser/metric.hpp"
	"${HEEL_HDR_PATH}/heel/parser/monitor.hpp"
	"${HEEL_HDR_PATH}/heel/parser/parameter.hpp"
	"${HEEL_HDR_PATH}/heel/parser/utils.hpp"
	"${HEEL_HDR_PATH}/heel/configuration_file.hpp"
	"${HEEL_HDR_PATH}/heel/json_parser.hpp"
	"${HEEL_HDR_PATH}/heel/print_application_model.hpp"
	"${HEEL_HDR_PATH}/heel/typer.hpp"
)
set( HEEL_LIBRARY_SOURCES
	"${HEEL_SRC_PATH}/configuration_file.cpp"
	"${HEEL_SRC_PATH}/generator_description_verbose.cpp"
	"${HEEL_SRC_PATH}/model_monitor.cpp"
	"${HEEL_SRC_PATH}/parser_agora.cpp"
	"${HEEL_SRC_PATH}/parser_application.cpp"
	"${HEEL_SRC_PATH}/parser_block.cpp"
	"${HEEL_SRC_PATH}/parser_features.cpp"
	"${HEEL_SRC_PATH}/parser_knob.cpp"
	"${HEEL_SRC_PATH}/parser_metric.cpp"
	"${HEEL_SRC_PATH}/parser_monitor.cpp"
	"${HEEL_SRC_PATH}/parser_parameter.cpp"
	"${HEEL_SRC_PATH}/parser_utils.cpp"
	"${HEEL_SRC_PATH}/typer.cpp"
)

# declare the target
add_library( ${HEEL_LIBRARY_NAME} STATIC ${HEEL_LIBRARY_SOURCES} ${HEEL_LIBRARY_HEADERS} )
add_library( margot::${HEEL_LIBRARY_NAME} ALIAS ${HEEL_LIBRARY_NAME} )

# define its interface
target_include_directories( ${HEEL_LIBRARY_NAME} PRIVATE ${HEEL_HDR_PATH} )
set_target_properties( ${HEEL_LIBRARY_NAME} PROPERTIES PUBLIC_HEADER "${HEEL_LIBRARY_HEADERS}" )

# set the executable dependencies
target_link_libraries( ${HEEL_LIBRARY_NAME} PUBLIC Boost::boost )

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
set( HEEL_BINARY_NAME "margot_heel")

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




#########################################################################
#### Define the install settings for all the stuff
#########################################################################

# copy library files
include(GNUInstallDirs)
install(TARGETS ${HEEL_LIBRARY_NAME} ${HEEL_BINARY_NAME}
		EXPORT ${HEEL_LIBRARY_NAME}Targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/heel
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/heel
)

# generate the config files to find the library
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${HEEL_LIBRARY_NAME}ConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
)
install(EXPORT ${HEEL_LIBRARY_NAME}Targets
    FILE ${HEEL_LIBRARY_NAME}Targets.cmake
    NAMESPACE margot::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${HEEL_LIBRARY_NAME}
)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/heelConfig.cmake.in ${HEEL_LIBRARY_NAME}Config.cmake @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${HEEL_LIBRARY_NAME}Config.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/${HEEL_LIBRARY_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${HEEL_LIBRARY_NAME}
        )        )

#########################################################################
#### Print a summary of the build configuration
#########################################################################

message ( STATUS "" )
message ( STATUS "//===-----------------------===//" )
message ( STATUS "//        CONFIG STATUS        //" )
message ( STATUS "//===-----------------------===//" )
message ( STATUS "Build directory.................: ${PROJECT_BINARY_DIR}" )
message ( STATUS "Install prefix..................: ${CMAKE_INSTALL_PREFIX}" )
message ( STATUS "Build type......................: ${CMAKE_BUILD_TYPE}")
message ( STATUS "//===-----------------------===//" )