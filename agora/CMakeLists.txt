#check the minimum version
cmake_minimum_required( VERSION 3.14 )

# the project name
project( AGORA VERSION 2.0 LANGUAGES CXX)

#########################################################################
#### Look for required external libraries
#########################################################################
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

find_package(eclipse-paho-mqtt-c REQUIRED)

find_package(Boost REQUIRED COMPONENTS program_options)

include(GNUInstallDirs)

#########################################################################
#### Define the Agora library target
#########################################################################

# define the target name and paths
set( AGORA_LIBRARY_NAME "agora_common")
set( AGORA_HDR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include" )
set( AGORA_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src")

# define the related headers and source files
set( AGORA_LIBRARY_HEADERS
    "${AGORA_HDR_PATH}/agora/application_handler.hpp"
    "${AGORA_HDR_PATH}/agora/application_manager.hpp"
    "${AGORA_HDR_PATH}/agora/agora_properties.hpp"
    "${AGORA_HDR_PATH}/agora/model_message.hpp"
    "${AGORA_HDR_PATH}/agora/model_doe.hpp"
    "${AGORA_HDR_PATH}/agora/model_cluster.hpp"
    "${AGORA_HDR_PATH}/agora/model_prediction.hpp"
    "${AGORA_HDR_PATH}/agora/logger.hpp"
    "${AGORA_HDR_PATH}/agora/logger_configuration.hpp"
    "${AGORA_HDR_PATH}/agora/logger_console.hpp"
    "${AGORA_HDR_PATH}/agora/logger_file.hpp"
    "${AGORA_HDR_PATH}/agora/fs_handler.hpp"
    "${AGORA_HDR_PATH}/agora/fs_configuration.hpp"
    "${AGORA_HDR_PATH}/agora/fs_description.hpp"
    "${AGORA_HDR_PATH}/agora/fs_doe.hpp"
    "${AGORA_HDR_PATH}/agora/fs_prediction.hpp"
    "${AGORA_HDR_PATH}/agora/fs_model.hpp"
    "${AGORA_HDR_PATH}/agora/fs_cluster.hpp"
    "${AGORA_HDR_PATH}/agora/fs_observation.hpp"
    "${AGORA_HDR_PATH}/agora/launcher.hpp"
    "${AGORA_HDR_PATH}/agora/launcher_configuration.hpp"
    "${AGORA_HDR_PATH}/agora/plugin_configuration.hpp"
    "${AGORA_HDR_PATH}/agora/remote_handler.hpp"
    "${AGORA_HDR_PATH}/agora/remote_configuration.hpp"
    "${AGORA_HDR_PATH}/agora/paho_remote_handler.hpp"
    "${AGORA_HDR_PATH}/agora/safe_queue.hpp"
    "${AGORA_HDR_PATH}/agora/threadpool.hpp"
    "${AGORA_HDR_PATH}/agora/worker.hpp"
    # csv_storage
    "${AGORA_HDR_PATH}/agora/csv/csv_fs_cluster.hpp"
    "${AGORA_HDR_PATH}/agora/csv/csv_fs_description.hpp"
    "${AGORA_HDR_PATH}/agora/csv/csv_fs_doe.hpp"
    "${AGORA_HDR_PATH}/agora/csv/csv_fs_observation.hpp"
    "${AGORA_HDR_PATH}/agora/csv/csv_fs_prediction.hpp"
    "${AGORA_HDR_PATH}/agora/csv/csv.hpp"
    #utils
    "${AGORA_HDR_PATH}/agora/utils/bitmask.hpp"
)

set( AGORA_LIBRARY_SOURCES
    "${AGORA_SRC_PATH}/application_handler.cpp"
    "${AGORA_SRC_PATH}/fs_handler.cpp"
    "${AGORA_SRC_PATH}/fs_description.cpp"
    "${AGORA_SRC_PATH}/fs_doe.cpp"
    "${AGORA_SRC_PATH}/fs_prediction.cpp"
    "${AGORA_SRC_PATH}/fs_model.cpp"
    "${AGORA_SRC_PATH}/fs_observation.cpp"
    "${AGORA_SRC_PATH}/fs_cluster.cpp"
    "${AGORA_SRC_PATH}/csv_fs_cluster.cpp"
    "${AGORA_SRC_PATH}/csv_fs_description.cpp"
    "${AGORA_SRC_PATH}/csv_fs_doe.cpp"
    "${AGORA_SRC_PATH}/csv_fs_observation.cpp"
    "${AGORA_SRC_PATH}/csv_fs_prediction.cpp"
    "${AGORA_SRC_PATH}/logger.cpp"
    "${AGORA_SRC_PATH}/launcher.cpp"
    "${AGORA_SRC_PATH}/remote_handler.cpp"
    "${AGORA_SRC_PATH}/paho_remote_handler.cpp"
    "${AGORA_SRC_PATH}/worker.cpp"
)

# declare the target
add_library( ${AGORA_LIBRARY_NAME} SHARED ${AGORA_LIBRARY_SOURCES} ${AGORA_LIBRARY_HEADERS} )
add_library( margot::${AGORA_LIBRARY_NAME} ALIAS ${AGORA_LIBRARY_NAME} )

# define its inerface
target_include_directories( ${AGORA_LIBRARY_NAME}
	PUBLIC
        $<BUILD_INTERFACE:${AGORA_HDR_PATH}> # for headers when building
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> # for clients in install mode
)

# set the executable dependencies
target_link_libraries( ${AGORA_LIBRARY_NAME} PUBLIC margot::margot_heel Threads::Threads eclipse-paho-mqtt-c::paho-mqtt3c Boost::headers)

# define its compiler options
target_compile_features( ${AGORA_LIBRARY_NAME} PRIVATE cxx_std_17 ) # for the filesystem component
target_compile_options( ${AGORA_LIBRARY_NAME} PRIVATE "-Wall" )
target_compile_options( ${AGORA_LIBRARY_NAME} PRIVATE $<$<CONFIG:DEBUG>: -g -O0 >)
target_compile_options( ${AGORA_LIBRARY_NAME} PRIVATE $<$<CONFIG:RELWITHDEBINFO>: -march=native -mtune=native -g -O2 >)
target_compile_options( ${AGORA_LIBRARY_NAME} PRIVATE $<$<CONFIG:RELEASE>: -march=native -mtune=native -O3 >)

#### UNIT TEST ####
#add_subdirectory(test)

#########################################################################
#### Agora server binary
#########################################################################
add_subdirectory(server)

#########################################################################
#### Define the install settings for all the stuff
#########################################################################

# install the targets generated
install(TARGETS ${AGORA_LIBRARY_NAME}
  EXPORT ${AGORA_LIBRARY_NAME}Targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(DIRECTORY ${AGORA_HDR_PATH}/agora DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# generate the config files to find the library and the executable
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${AGORA_LIBRARY_NAME}ConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY ExactVersion
)
install(EXPORT ${AGORA_LIBRARY_NAME}Targets
  FILE ${AGORA_LIBRARY_NAME}Targets.cmake
    NAMESPACE margot::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/margot
)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/agora_commonConfig.cmake.in ${AGORA_LIBRARY_NAME}Config.cmake @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${AGORA_LIBRARY_NAME}Config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/${AGORA_LIBRARY_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/margot
        )
