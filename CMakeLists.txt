# check the minimum version
cmake_minimum_required( VERSION 3.0.2 )

# the project name
project( mARGOt )

# set the version

set( PROJECT_VERSION_MAJOR 2 )
set( PROJECT_VERSION_MINOR 0 )

################################
#### General configure section
################################

# force the Release build if not already set
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif(NOT CMAKE_BUILD_TYPE)

# setting common c++ flags
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x" )

# setting debug flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -g3 -O0")

# setting release with debug info flags
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -march=native -mtune=native -g3 -O2")

# setting release flags
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -march=native -mtune=native -O3")



################################
#### Option section
################################

option( LIB_STATIC "Build a static version of the library" ON)

option( WITH_TEST "Build a test unit application" ON )
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

option( USE_PAPI_MONITOR "Build the PAPI monitor" ON )
if ( USE_PAPI_MONITOR )
	list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/framework/cmake")
	find_package( PAPI )
endif( USE_PAPI_MONITOR )

option( USE_TEMPERATURE_MONITOR "Build the temperature monitor" ON )
if ( USE_TEMPERATURE_MONITOR )
	list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/framework/cmake")
	find_package( SENSORS )
endif( USE_TEMPERATURE_MONITOR )



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


# this message is just for pretty printing
message( STATUS "" )




################################
#### Build section
################################

# build the framework
add_subdirectory( framework )

# generate auto complete file
set( AUTOCOMPLETE_FILE_NAME .clang_complete )
file( WRITE ${AUTOCOMPLETE_FILE_NAME} "-I${PROJECT_SOURCE_DIR}/framework/include\n" )
if ( WITH_TEST )
	file( APPEND ${AUTOCOMPLETE_FILE_NAME} "-I${CXXTEST_INCLUDES}" )
endif ( WITH_TEST )
if ( HAVE_COLLECTOR )
	file( APPEND ${AUTOCOMPLETE_FILE_NAME} "-I${COLLECTOR_INCLUDES}" )
endif ( HAVE_COLLECTOR )
if ( HAVE_PAPI )
	file( APPEND ${AUTOCOMPLETE_FILE_NAME} "-I${PAPI_INCLUDES}" )
endif ( HAVE_PAPI )
if ( HAVE_SENSORS )
	file( APPEND ${AUTOCOMPLETE_FILE_NAME} "-I${SENSORS_INCLUDES}" )
endif ( HAVE_SENSORS )
