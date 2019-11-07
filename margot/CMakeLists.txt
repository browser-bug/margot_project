# check the minimum version
cmake_minimum_required( VERSION 3.4.3 ) # for the imported targets

# the project name
project( core VERSION 3.0 LANGUAGES CXX)


#########################################################################
#### Add further option to drive the compilation of the mARGOt library
#########################################################################

option( LIB_STATIC "Build a static version of the mARGOt library" ON)
option( GEN_DOC "Generate the mARGOt Doxygen documentation" OFF)
option( WITH_TEST "Build a test unit application for mARGOt" OFF)
option( WITH_BENCHMARK "Build a benchmark to evaluate the mARGOt overheads" OFF)
option( WITH_COLLECTOR_MONITOR "Build the collector monitor" OFF)
option( WITH_PAPI_MONITOR "Build the PAPI monitor" OFF)
option( WITH_TEMPERATURE_MONITOR "Build the temperature monitor" OFF)


#########################################################################
#### Look for required external libraries, according to the options
#########################################################################

# add the path to the custom finders in the mARGOt repository
set( MARGOT_CUSTOM_FINDERS_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list (APPEND CMAKE_MODULE_PATH "${MARGOT_CUSTOM_FINDERS_PATH}")
message(STATUS "${CMAKE_INSTALL_PREFIX}")

# define a macro to try to use the internal built version fo stuff (if not alredy set)
macro( set_gentle_hint TARGET_ROOT_VARIABLE )
	if (NOT DEFINED ${TARGET_ROOT_VARIABLE})
		if (DEFINED ENV{${TARGET_ROOT_VARIABLE}})
			set( ${TARGET_ROOT_VARIABLE} "$ENV{${TARGET_ROOT_VARIABLE}}" )
		else ()
			set( ${TARGET_ROOT_VARIABLE} "${CMAKE_INSTALL_PREFIX}" )
		endif ()
	endif()
endmacro(set_gentle_hint)

# check for all the mandatory dependencies
find_package( Threads REQUIRED )
set_gentle_hint(OPENSSL_ROOT_DIR)
find_package( OpenSSL REQUIRED )
set_gentle_hint(PAHOMQTT_ROOT_DIR)
find_package( PahoMQTT REQUIRED )


# conditional test according to the build options
if ( WITH_TEST )
	find_package( CxxTest REQUIRED)
endif ( WITH_TEST )
if ( GEN_DOC )
	find_package( Doxygen REQUIRED)
endif ( GEN_DOC )
if ( WITH_COLLECTOR_MONITOR )
	set_gentle_hint(COLLECTOR_ROOT_DIR)
	find_package( COLLECTOR REQUIRED)
endif ( WITH_COLLECTOR_MONITOR )
if ( WITH_PAPI_MONITOR )
	set_gentle_hint(PFM_ROOT_DIR)
	find_package( PFM REQUIRED)
	set_gentle_hint(PAPI_ROOT_DIR)
	find_package( PAPI REQUIRED)
endif( WITH_PAPI_MONITOR )
if ( WITH_TEMPERATURE_MONITOR )
	set_gentle_hint(SENSORS_ROOT_DIR)
	find_package( SENSORS REQUIRED)
endif( WITH_TEMPERATURE_MONITOR )



#########################################################################
#### Define the mARGOt framework target
#########################################################################

# define the library name and paths
set( MARGOT_LIBRARY_NAME "margot_core")
set( MARGOT_HDR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include" )
set( MARGOT_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src")


# set the headers and source files of the mARGOt library
set( MARGOT_HEADERS
	"${MARGOT_HDR_PATH}/margot/basic_information_block.hpp"
	"${MARGOT_HDR_PATH}/margot/hash.hpp"
	"${MARGOT_HDR_PATH}/margot/operating_point.hpp"
	"${MARGOT_HDR_PATH}/margot/operating_point_segment.hpp"
	"${MARGOT_HDR_PATH}/margot/traits.hpp"
	"${MARGOT_HDR_PATH}/margot/knowledge_base.hpp"
	"${MARGOT_HDR_PATH}/margot/circular_buffer.hpp"
	"${MARGOT_HDR_PATH}/margot/statistical_provider.hpp"
	"${MARGOT_HDR_PATH}/margot/statistics.hpp"
	"${MARGOT_HDR_PATH}/margot/enums.hpp"
	"${MARGOT_HDR_PATH}/margot/field_adaptor.hpp"
	"${MARGOT_HDR_PATH}/margot/knowledge_adaptor.hpp"
	"${MARGOT_HDR_PATH}/margot/evaluator.hpp"
	"${MARGOT_HDR_PATH}/margot/view.hpp"
	"${MARGOT_HDR_PATH}/margot/monitor.hpp"
	"${MARGOT_HDR_PATH}/margot/rank.hpp"
	"${MARGOT_HDR_PATH}/margot/goal.hpp"
	"${MARGOT_HDR_PATH}/margot/constraint.hpp"
	"${MARGOT_HDR_PATH}/margot/state.hpp"
	"${MARGOT_HDR_PATH}/margot/asrtm.hpp"
	"${MARGOT_HDR_PATH}/margot/energy_monitor.hpp"
	"${MARGOT_HDR_PATH}/margot/frequency_monitor.hpp"
	"${MARGOT_HDR_PATH}/margot/memory_monitor.hpp"
	"${MARGOT_HDR_PATH}/margot/odroid_energy_monitor.hpp"
	"${MARGOT_HDR_PATH}/margot/odroid_power_monitor.hpp"
	"${MARGOT_HDR_PATH}/margot/process_cpu_usage_monitor.hpp"
	"${MARGOT_HDR_PATH}/margot/system_cpu_usage_monitor.hpp"
	"${MARGOT_HDR_PATH}/margot/throughput_monitor.hpp"
	"${MARGOT_HDR_PATH}/margot/time_monitor.hpp"
	"${MARGOT_HDR_PATH}/margot/da_asrtm.hpp"
	"${MARGOT_HDR_PATH}/margot/debug.hpp"
	"${MARGOT_HDR_PATH}/margot/data_features.hpp"
	"${MARGOT_HDR_PATH}/margot/virtual_channel.hpp"
	"${MARGOT_HDR_PATH}/margot/virtual_channel_impl_paho.hpp"
	"${MARGOT_HDR_PATH}/margot/virtual_channel_interface.hpp"
	)
set( SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src" )
set( MARGOT_SOURCES
	"${MARGOT_SRC_PATH}/energy_monitor"
	"${MARGOT_SRC_PATH}/frequency_monitor"
	"${MARGOT_SRC_PATH}/memory_monitor"
	"${MARGOT_SRC_PATH}/odroid_power_monitor"
	"${MARGOT_SRC_PATH}/odroid_energy_monitor"
	"${MARGOT_SRC_PATH}/process_cpu_usage_monitor"
	"${MARGOT_SRC_PATH}/system_cpu_usage_monitor"
	"${MARGOT_SRC_PATH}/throughput_monitor"
	"${MARGOT_SRC_PATH}/time_monitor"
	"${MARGOT_SRC_PATH}/virtual_channel_impl_paho"
)

# add the target
if ( LIB_STATIC )
	add_library( ${MARGOT_LIBRARY_NAME} STATIC ${MARGOT_SOURCES} )
else ( LIB_STATIC )
	add_library( ${MARGOT_LIBRARY_NAME} SHARED ${MARGOT_SOURCES} )
endif ( LIB_STATIC )
add_library(margot::${MARGOT_LIBRARY_NAME} ALIAS ${MARGOT_LIBRARY_NAME})

# define its interface
target_include_directories( ${MARGOT_LIBRARY_NAME}
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${MARGOT_HDR_PATH}>
)
set_target_properties(${MARGOT_LIBRARY_NAME} PROPERTIES PUBLIC_HEADER "${MARGOT_HEADERS}" )

# define the linker options
target_link_libraries(${MARGOT_LIBRARY_NAME} PUBLIC
        PahoMQTT::PahoMQTT
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads
)


# also according to the target
if ( WITH_COLLECTOR_MONITOR )
	target_link_libraries( ${MARGOT_LIBRARY_NAME} PUBLIC Collector::Collector )
endif ( WITH_COLLECTOR_MONITOR )
if ( WITH_PAPI_MONITOR )
	target_link_libraries( ${MARGOT_LIBRARY_NAME} PUBLIC Papi::Papi )
endif( WITH_PAPI_MONITOR )
if ( WITH_TEMPERATURE_MONITOR )
	target_link_libraries( ${MARGOT_LIBRARY_NAME} PUBLIC Sensors::Sensors )
endif( WITH_TEMPERATURE_MONITOR )



#########################################################################
#### Setting the compiler options according to the build type
#########################################################################

# to compile the application we need at least C++11
target_compile_features( ${MARGOT_LIBRARY_NAME} PRIVATE cxx_std_11 )

# turn on compiler warings regardless of the build type
target_compile_options(${MARGOT_LIBRARY_NAME} PRIVATE "-Wall")

# now we can define compiler options for each build target
target_compile_options(${MARGOT_LIBRARY_NAME} PRIVATE $<$<CONFIG:DEBUG>: -g -O0 >)
target_compile_options(${MARGOT_LIBRARY_NAME} PRIVATE $<$<CONFIG:RELWITHDEBINFO>: -march=native -mtune=native -g -O2 >)
target_compile_options(${MARGOT_LIBRARY_NAME} PRIVATE $<$<CONFIG:RELEASE>: -march=native -mtune=native -O3 >)



#########################################################################
#### Define the install settings
#########################################################################

include(GNUInstallDirs)
install(TARGETS ${MARGOT_LIBRARY_NAME}
	EXPORT ${MARGOT_LIBRARY_NAME}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/margot
)
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    ${MARGOT_LIBRARY_NAME}ConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY ExactVersion
)
install(EXPORT ${MARGOT_LIBRARY_NAME}Targets
    FILE ${MARGOT_LIBRARY_NAME}Targets.cmake
    NAMESPACE margot::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/margot
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${MARGOT_LIBRARY_NAME}Config.cmake"
              "${CMAKE_CURRENT_BINARY_DIR}/${MARGOT_LIBRARY_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/margot
        )
install(FILES "${MARGOT_CUSTOM_FINDERS_PATH}/FindCOLLECTOR.cmake"
	          "${MARGOT_CUSTOM_FINDERS_PATH}/FindPahoMQTT.cmake"
	          "${MARGOT_CUSTOM_FINDERS_PATH}/FindPAPI.cmake"
	          "${MARGOT_CUSTOM_FINDERS_PATH}/FindPFM.cmake"
	          "${MARGOT_CUSTOM_FINDERS_PATH}/FindSENSORS.cmake"
	    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/margot
	    )
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config/margotConfig.cmake.in ${MARGOT_LIBRARY_NAME}Config.cmake @ONLY)

#########################################################################
#### Build additional stuff with the mARGOt autotuning framework
#########################################################################

if ( WITH_TEST )
	add_subdirectory( test )
endif ( WITH_TEST )

if ( WITH_BENCHMARK )
	add_subdirectory( benchmark )
endif ( WITH_BENCHMARK )

if ( GEN_DOC )
	add_subdirectory( doc )
endif ( GEN_DOC )


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
message ( STATUS "Static library...................: ${LIB_STATIC} (CMake option LIB_STATIC)" )
message ( STATUS "Build test application...........: ${WITH_TEST} (CMake option WITH_TEST)" )
message ( STATUS "Build benchmark application......: ${WITH_BENCHMARK} (CMake option WITH_BENCHMARK)" )
message ( STATUS "Generate Doxygen documentation...: ${GEN_DOC} (CMake option GEN_DOC)" )
message ( STATUS "With PAPI monitors...............: ${WITH_PAPI_MONITOR} (CMake option WITH_PAPI_MONITOR)" )
message ( STATUS "With temperature monitors........: ${WITH_TEMPERATURE_MONITOR} (CMake option WITH_TEMPERATURE_MONITOR)")
message ( STATUS "//===------------------------===//" )
message ( STATUS "" )