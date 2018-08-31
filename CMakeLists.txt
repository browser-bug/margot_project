# check the minimum version
cmake_minimum_required( VERSION 2.8.12 )

# the project name
project( mARGOt )

# set the version

set( PROJECT_VERSION_MAJOR 2 )
set( PROJECT_VERSION_MINOR 0 )



################################
#### Option section
################################

option( LIB_STATIC "Build a static version of the library" ON)

option( WITH_TEST "Build a test unit application" OFF )
if ( WITH_TEST )
	list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/framework/test/cmake")
	find_package( CXXTEST )
endif ( WITH_TEST )

option( GEN_DOC "Generate a Doxygen documentation" OFF )
if ( GEN_DOC )
	find_package( Doxygen )
endif ( GEN_DOC )

option( USE_COLLECTOR_MONITOR "Build the collector monitor" OFF )
if ( USE_COLLECTOR_MONITOR )
	list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/framework/cmake")
	find_package( COLLECTOR )
endif ( USE_COLLECTOR_MONITOR )

option( USE_PAPI_MONITOR "Build the PAPI monitor" OFF )
if ( USE_PAPI_MONITOR )
	list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/framework/cmake")
	find_package( PAPI )
endif( USE_PAPI_MONITOR )

option( USE_TEMPERATURE_MONITOR "Build the temperature monitor" OFF )
if ( USE_TEMPERATURE_MONITOR )
	list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/framework/cmake")
	find_package( SENSORS )
endif( USE_TEMPERATURE_MONITOR )

option( WITH_CASSANDRA "Enable the cassandra storage back-end" OFF )

option( WITH_AGORA "With agora: the server-side application to manage appication knowledge" OFF )
if ( WITH_AGORA )
	list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/framework/cmake")
	find_package( MQTT )
	if ( WITH_CASSANDRA )
		find_package( CASSANDRA )
	endif ( WITH_CASSANDRA )
endif( WITH_AGORA )


option( WITH_BENCHMARK "Build a benchmark to evaluate the overheads" OFF)



################################
#### General configure section
################################

# force the Release build if not already set
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif(NOT CMAKE_BUILD_TYPE)

# setting common c++ flags
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x" )
if ( WITH_CASSANDRA )
	set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -DAGORA_ENABLE_CASSANDRA_STORAGE" )
endif ( WITH_CASSANDRA )

# setting debug flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g3 -O0")

# setting release with debug info flags
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -march=native -mtune=native -g3 -O2")

# setting release flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -mtune=native -O3")



################################
#### Dependency check
################################

# print preamble
message( STATUS "" )
message( STATUS "*****************************************" )
message( STATUS "**            CONFIG STATUS            **" )
message( STATUS "*****************************************" )


# check if we must compile the library in a static way
if ( LIB_STATIC )
	message( STATUS "Build library........................: STATIC (-DLIB_STATIC=OFF for dynamic)" )
else ( LIB_STATIC )
	message( STATUS "Build library........................: DYNAMIC (-DLIB_STATIC=ON for static)" )
endif ( LIB_STATIC )


# check if we have to generate the test application
if ( WITH_TEST )
	if ( HAVE_CXXTEST )
		message( STATUS "Test application.....................: ENABLED (-DWITH_TEST=OFF to disable)" )
	else ( HAVE_CXXTEST )
		message( SEND_ERROR "Test application.....................: CXXTEST not found!" )
		message( "    POSSIBLE SOLUTIONS:")
		message( "    1) Install cxxtest system-wide")
		message( "    2) Export cxxtest root path in CXXTEST_ROOT")
		message( "       environmental variable (current value = \"$ENV{CXXTEST_ROOT}\")")
		message( "    3) Disable the option with -DWITH_TEST=OFF\n\n\n")
		return()
	endif ( HAVE_CXXTEST)
else ( WITH_TEST )
	message( STATUS "Test application.....................: DISABLED (-DWITH_TEST=ON to enable)" )
endif ( WITH_TEST )


# check if we have to generate the doxygen documentation
if ( GEN_DOC )
	if ( DOXYGEN_FOUND )
		message( STATUS "Generate documentation...............: ENABLED (-DGEN_DOC=OFF to disable)" )
	else ( DOXYGEN_FOUND )
		message( SEND_ERROR "Generate documentation...............: DOXYGEN not found!" )
		message( "    POSSIBLE SOLUTIONS:")
		message( "    1) Install doxygen system-wide")
		message( "    3) Disable the option with -DGEN_DOC=OFF\n\n\n")
		return()
	endif ( DOXYGEN_FOUND)
else ( GEN_DOC )
	message( STATUS "Generate documentation...............: DISABLED (-DGEN_DOC=ON to enable)" )
endif ( GEN_DOC )


# check if we can compile the collector monitor
if ( USE_COLLECTOR_MONITOR )
	if ( HAVE_COLLECTOR )
		message( STATUS "Using collector monitor..............: ENABLED (-DUSE_COLLECTOR_MONITOR=OFF to disable)" )
	else ( HAVE_COLLECTOR )
		message( SEND_ERROR "Using collector monitor..............: COLLECTOR not found!" )
		message( "    POSSIBLE SOLUTIONS:")
		message( "    1) Install collector system-wide")
		message( "    2) Export collector root path in COLECTOR_ROOT")
		message( "       environmental variable (current value = \"$ENV{COLLECTOR_ROOT}\")")
		message( "    3) Disable the option with -DUSE_COLLECTOR_MONITOR=OFF\n\n\n")
		return()
	endif ( HAVE_COLLECTOR)
else ( USE_COLLECTOR_MONITOR )
	message( STATUS "Using collector monitor..............: DISABLED (-DUSE_COLLECTOR_MONITOR=ON to enable)" )
endif ( USE_COLLECTOR_MONITOR )


# check if we can compile the collector monitor
if ( USE_PAPI_MONITOR )
	if ( HAVE_PAPI )
		message( STATUS "Using papi monitor...................: ENABLED (-DUSE_PAPI_MONITOR=OFF to disable)" )
	else ( HAVE_PAPI )
		message( SEND_ERROR "Using papi monitor...................: PAPI not found!" )
		message( "    POSSIBLE SOLUTIONS:")
		message( "    1) Install papi system-wide")
		message( "    2) Export papi root path in PAPI_ROOT")
		message( "       environmental variable (current value = \"$ENV{PAPI_ROOT}\")")
		message( "    3) Disable the option with -DUSE_PAPI_MONITOR=OFF\n\n\n")
		return()
	endif ( HAVE_PAPI)
else ( USE_PAPI_MONITOR )
	message( STATUS "Using papi monitor...................: DISABLED (-DUSE_PAPI_MONITOR=ON to enable)" )
endif ( USE_PAPI_MONITOR )


# check if we can compile the collector monitor
if ( USE_TEMPERATURE_MONITOR )
	if ( HAVE_SENSORS )
		message( STATUS "Using temperature monitor............: ENABLED (-DUSE_TEMPERATURE_MONITOR=OFF to disable)" )
	else ( HAVE_SENSORS )
		message( SEND_ERROR "Using temperature monitor............: SENSORS not found!" )
		message( "    POSSIBLE SOLUTIONS:")
		message( "    1) Install sensors system-wide")
		message( "    2) Export sensors root path in SENSORS_ROOT")
		message( "       environmental variable (current value = \"$ENV{SENSORS_ROOT}\")")
		message( "    3) Disable the option with -DUSE_TEMPERATURE_MONITOR=OFF\n\n\n")
		return()
	endif ( HAVE_SENSORS)
else ( USE_TEMPERATURE_MONITOR )
	message( STATUS "Using temperature monitor............: DISABLED (-DUSE_TEMPERATURE_MONITOR=ON to enable)" )
endif ( USE_TEMPERATURE_MONITOR )


# check if we compile the cassandra storage back end
if ( WITH_CASSANDRA AND WITH_AGORA )
	message( STATUS "Cassandra storage....................: ENABLED (-DWITH_CASSANDRA=OFF to disable)" )
	if ( NOT HAVE_CASSANDRA )
		include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

		# print the fact that we fetch and build an external library
		message( STATUS "    NOTE: CASSANDRA driver not found, we build it internally, internet connection required")
		message( STATUS "          it requires also OpenSLL and libvu components, they are not internally built")

		# set up the mqtt environment
		set( CASSANDRA_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/extern/install)
		ExternalProject_add( CASSANDRA
			PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/extern/cassandra
			GIT_REPOSITORY https://github.com/datastax/cpp-driver.git
			GIT_TAG 2.8.1
			UPDATE_COMMAND ""
			PATCH_COMMAND ""
			CMAKE_ARGS -DCASS_BUILD_STATIC:STRING=TRUE -DCASS_BUILD_SHARED:STRING=FALSE -DCASS_USE_STATIC_LIBS:STRING=TRUE -DCMAKE_INSTALL_PREFIX:PATH=${CASSANDRA_ROOT} -DCMAKE_BUILD_TYPE:STRING=RELEASE
		)
		set( HAVE_CASSANDRA TRUE)
		set( CASSANDRA_LIBRARY ${CASSANDRA_ROOT}/lib/libcassandra_static.a)
		set( CASSANDRA_INCLUDES ${CASSANDRA_ROOT}/include)
		find_package ( LIBUV )
		find_package ( OpenSSL )
		set ( CASSANDRA_LIBRARIES ${CASSANDRA_LIBRARY} ${LIBUV_LIBRARIES} ${OPENSSL_LIBRARIES} )
	endif ( NOT HAVE_CASSANDRA )

	include_directories( ${CASSANDRA_INCLUDES})

else ( WITH_CASSANDRA AND WITH_AGORA )
	if ( WITH_CASSANDRA )
		message( STATUS "Cassandra storage....................: DISABLED ( it requires agora )" )
	else ( WITH_CASSANDRA )
		message( STATUS "Cassandra storage....................: DISABLED (-DWITH_CASSANDRA=ON to enable)" )
	endif ( WITH_CASSANDRA )
endif ( WITH_CASSANDRA AND WITH_AGORA )

# check if we can compile margot agora
if ( WITH_AGORA )
	message( STATUS "Server application agora.............: ENABLED (-DWITH_AGORA=OFF to disable)" )

	# check if we have to build some component manually
	if ( NOT HAVE_MQTT )
		include(${CMAKE_ROOT}/Modules/ExternalProject.cmake)

		# print the fact that we fetch and build an external library
		message( STATUS "    NOTE: MQTT not found, we build it internally, internet connection required")

		# set up the mqtt environment
		set( MQTT_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/extern/install)
		ExternalProject_add( MQTT
			PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/extern/mqtt
			GIT_REPOSITORY https://github.com/eclipse/paho.mqtt.c.git
			GIT_TAG v1.2.0
			UPDATE_COMMAND ""
			PATCH_COMMAND ""
			CMAKE_ARGS -DPAHO_BUILD_STATIC:STRING=TRUE -DCMAKE_INSTALL_PREFIX:PATH=${MQTT_ROOT} -DCMAKE_BUILD_TYPE:STRING=RELEASE -DPAHO_WITH_SSL=TRUE
		)
		find_library (MQTT_PTHREADS pthread)
		set( HAVE_MQTT TRUE)
		set( MQTT_LIBRARIES ${MQTT_ROOT}/lib/libpaho-mqtt3cs-static.a ${MQTT_PTHREADS})
		set( MQTT_INCLUDES ${MQTT_ROOT}/include)
	endif ( NOT HAVE_MQTT )

	# eventually include the headers directory
	include_directories( ${MQTT_INCLUDES})
else ( WITH_AGORA )
	message( STATUS "Server application agora.............: DISABLED (-DWITH_AGORA=ON to enable)" )
endif ( WITH_AGORA )


# check if we must compile the benchmark
if ( WITH_BENCHMARK )
	message( STATUS "Build framework benchmark............: ENABLED (-DWITH_BENCHMARK=OFF to disable)" )
else ( WITH_BENCHMARK )
	message( STATUS "Build framework benchmark............: DISABLED (-DWITH_BENCHMARK=ON to enable)" )
endif ( WITH_BENCHMARK )


# this message is just for pretty printing
message( STATUS "" )




################################
#### Build section
################################

# set the name of the margot library
set( MARGOT_LIB_NAME "margot")

# set the position of the margot headers
set( MARGOT_HDR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/framework/include" )
set( MARGOT_CONFIG_FILE_PATH "${PROJECT_BINARY_DIR}/include" )
include_directories( ${MARGOT_HDR_PATH} ${MARGOT_CONFIG_FILE_PATH} )

# build the framework
add_subdirectory( framework )

# build the remote application handler
if ( WITH_AGORA )
	add_subdirectory( agora )
endif ( WITH_AGORA)

# build the benchmark
if ( WITH_BENCHMARK )
	add_subdirectory( extra/benchmark )
endif ( WITH_BENCHMARK )

# generate auto complete file
set( AUTOCOMPLETE_FILE_NAME .clang_complete )
file( WRITE ${AUTOCOMPLETE_FILE_NAME} "-I${PROJECT_SOURCE_DIR}/framework/include\n" )
file( APPEND ${AUTOCOMPLETE_FILE_NAME} "-I${MARGOT_CONFIG_FILE_PATH}\n" )
if ( WITH_AGORA )
	file( APPEND ${AUTOCOMPLETE_FILE_NAME} "-I${MQTT_INCLUDES}\n" )
	file( APPEND ${AUTOCOMPLETE_FILE_NAME} "-I${CASSANDRA_INCLUDES}\n" )
endif ( WITH_AGORA )
if ( WITH_TEST )
	file( APPEND ${AUTOCOMPLETE_FILE_NAME} "-I${CXXTEST_INCLUDES}\n" )
endif ( WITH_TEST )
if ( HAVE_COLLECTOR )
	file( APPEND ${AUTOCOMPLETE_FILE_NAME} "-I${COLLECTOR_INCLUDES}\n" )
endif ( HAVE_COLLECTOR )
if ( HAVE_PAPI )
	file( APPEND ${AUTOCOMPLETE_FILE_NAME} "-I${PAPI_INCLUDES}\n" )
endif ( HAVE_PAPI )
if ( HAVE_SENSORS )
	file( APPEND ${AUTOCOMPLETE_FILE_NAME} "-I${SENSORS_INCLUDES}\n" )
endif ( HAVE_SENSORS )
if ( HAVE_MQTT )
	file( APPEND ${AUTOCOMPLETE_FILE_NAME} "-I${MQTT_INCLUDES}\n" )
endif ( HAVE_MQTT )
