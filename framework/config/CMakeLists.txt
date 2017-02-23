###############################################
##         Configuration section             ##
###############################################


# check for the clock_gettime function
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set( WITH_HARDWARE_COUNTER ON )
else (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	set( WITH_HARDWARE_COUNTER OFF )
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# check for nullptr support
set( WITH_NULLPTR  ON )
if(CMAKE_COMPILER_IS_GNUCC)
    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                    OUTPUT_VARIABLE GCC_VERSION)
    string(REGEX MATCHALL "[0-9]+" GCC_VERSION_COMPONENTS ${GCC_VERSION})

    if( NOT (${GCC_VERSION} VERSION_LESS 4.6 ) )
        set( WITH_NULLPTR  ON )
    else( NOT (${GCC_VERSION} VERSION_LESS 4.6 ) )
        set( WITH_NULLPTR  OFF )
    endif( NOT (${GCC_VERSION} VERSION_LESS 4.6 ) )
endif(CMAKE_COMPILER_IS_GNUCC)

# check for steady_clock_support
set( WITH_STEADY_CLOCK  ON )
if(CMAKE_COMPILER_IS_GNUCC)
    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                    OUTPUT_VARIABLE GCC_VERSION)
    string(REGEX MATCHALL "[0-9]+" GCC_VERSION_COMPONENTS ${GCC_VERSION})

    if( NOT (${GCC_VERSION} VERSION_LESS 4.7 ) )
        set( WITH_STEADY_CLOCK  ON )
    else( NOT (${GCC_VERSION} VERSION_LESS 4.7 ) )
        set( WITH_STEADY_CLOCK  OFF )
    endif( NOT (${GCC_VERSION} VERSION_LESS 4.7 ) )
endif(CMAKE_COMPILER_IS_GNUCC)

# check for the papi framework
if( HAVE_PAPI )
        set( WITH_PAPI  ON )
else( HAVE_PAPI )
        set( WITH_PAPI  OFF )
endif( HAVE_PAPI )

# check for the sensors library
if( HAVE_SENSORS )
        set( WITH_SENSORS  ON )
else( HAVE_SENSORS )
        set( WITH_SENSORS  OFF )
endif( HAVE_SENSORS )





###############################################
##           Configuration file              ##
###############################################


message ( STATUS "" )
message ( STATUS "*********************************" )
message ( STATUS "**        CONFIG STATUS        **" )
message ( STATUS "*********************************" )
message ( STATUS "Install prefix..................: ${CMAKE_INSTALL_PREFIX}" )


# --- print the result of the configuration
if( LIB_STATIC )
        message ( STATUS "Library type....................: STATIC" )
else( LIB_STATIC )
        message ( STATUS "Library type....................: DYNAMIC" )
endif( LIB_STATIC )

if ( SAFE_CASTING )

    # FIXME properly check if RTTI is supported by the compiler
    set( RTTI_ENABLED TRUE)

    if ( RTTI_ENABLED )
        message ( STATUS "Safe casting....................: ENABLED" )
        set( MARGOT_SAFE_CASTING "true")
    else( RTTI_ENABLED )
        message ( STATUS "Safe casting....................: DISABLED -> RTTI NOT SUPPORTED" )
        set( MARGOT_SAFE_CASTING "false")
    endif( RTTI_ENABLED )
else( SAFE_CASTING )
        message ( STATUS "Safe casting....................: DISABLED" )
        set( MARGOT_SAFE_CASTING "false")
endif( SAFE_CASTING )

if ( PEDANTIC_CHECK )
        message ( STATUS "Pedantic controls...............: ENABLED" )
        set( MARGOT_PEDANTIC_CHECKS "true")
else( PEDANTIC_CHECK )
        message ( STATUS "Pedantic controls...............: DISABLED" )
        set( MARGOT_PEDANTIC_CHECKS "true")
endif( PEDANTIC_CHECK )


# check if building the documentation
if ( WITH_DOC )

        # check for the doxygen executable
        if ( DOXYGEN_FOUND )
            message ( STATUS "Oxygen documentatation..........: ENABLED" )
        else ( DOXYGEN_FOUND )
            message ( FATAL_ERROR "Oxygen documentatation..........: DOXYGEN NOT FOUND! ( please, install doxygen )" )
        endif ( DOXYGEN_FOUND )

else( WITH_DOC )
        message ( STATUS "Oxygen documentatation..........: DISABLED" )
endif( WITH_DOC )


# check if the reward computation is enabled
if ( ENABLE_REWARD OR ENABLE_LEARNING )
        if ( (NOT ENABLE_REWARD) AND ENABLE_LEARNING )
                message ( STATUS "Reward computation..............: ENABLED (As Dependency)" )
        else ( (NOT ENABLE_REWARD) AND ENABLE_LEARNING )
                message ( STATUS "Reward computation..............: ENABLED" )
        endif( (NOT ENABLE_REWARD) AND ENABLE_LEARNING )
        set( MARGOT_ENABLE_REWARD_COMPUTATION ON )
else ( ENABLE_REWARD OR ENABLE_LEARNING )
        message ( STATUS "Reward computation..............: DISABLED" )
        set( MARGOT_ENABLE_REWARD_COMPUTATION OFF )
endif ( ENABLE_REWARD OR ENABLE_LEARNING )


# check if the logging of the learning is enabled
if ( LOG_LEARNING )
        message ( STATUS "Log learing (DEBUG).............: ENABLED" )
        set( MARGOT_LEARNING_ENABLE_FILE_LOG ON )
else ( LOG_LEARNING )
        message ( STATUS "Log learing (DEBUG).............: DISABLED" )
        set( MARGOT_LEARNING_ENABLE_FILE_LOG OFF )
endif ( LOG_LEARNING )

# check if we enable the learning state
if ( ENABLE_LEARNING )
        message ( STATUS "Enable Leaning..................: ENABLED" )
        set( MARGOT_LEARNING_ENABLE_STATE ON )
else ( ENABLE_LEARNING )
        message ( STATUS "Enable Leaning..................: DISABLED" )
        set( MARGOT_LEARNING_ENABLE_STATE OFF )
endif ( ENABLE_LEARNING )


# check if we are going to build the test executable
if ( CXXTEST_FOUND )
        message ( STATUS  "Framework test executable.......: ENABLED" )
else ( CXXTEST_FOUND )
        message ( STATUS  "Framework test executable.......: CXXTEST NOT FOUND! ( please, install cxxtest )" )
endif ( CXXTEST_FOUND )


# check if clock_gettime is available
if( WITH_HARDWARE_COUNTER )
        message ( STATUS  "clock_gettime...................: AVAILABLE" )
else( WITH_HARDWARE_COUNTER )
        message ( STATUS  "clock_gettime...................: UNAVAILABLE" )
endif( WITH_HARDWARE_COUNTER )


# check for the support of nullptr
if( WITH_NULLPTR )
        message ( STATUS  "nullptr ........................: AVAILABLE" )
else(WITH_NULLPTR )
        message ( STATUS  "nullptr ........................: UNAVAILABLE" )
endif( WITH_NULLPTR )


# check for the support of steady_clock
if( WITH_STEADY_CLOCK )
        message ( STATUS  "steady_clock ...................: AVAILABLE" )
else( WITH_STEADY_CLOCK )
        message ( STATUS  "steady_clock ...................: UNAVAILABLE" )
endif( WITH_STEADY_CLOCK )


# check for the PAPI libraries
if( HAVE_PAPI )
        message ( STATUS  "PAPI framework..................: AVAILABLE" )
else( HAVE_PAPI )
        message ( STATUS  "PAPI framework..................: PAPI NOT FOUND! ( please, install papi )" )
endif( HAVE_PAPI )


# check for the SENSORS libraries
if( HAVE_SENSORS )
        message ( STATUS  "SENSORS library.................: AVAILABLE" )
else( HAVE_SENSORS )
        message ( STATUS  "SENSORS library.................: SENSORS NOT FOUND! ( please, install sensors )" )
endif( HAVE_SENSORS )


# check for the COLLECTOR libraries
if( HAVE_COLLECTOR )
        message ( STATUS  "Collector framework.............: AVAILABLE" )
else( HAVE_COLLECTOR )
        message ( STATUS  "Collector framework.............: COLLECTOR NOT FOUND! ( please, export COLLECTOR_ROOT )" )
endif( HAVE_COLLECTOR )



message ( STATUS "" )





###############################################
##         Configuration section             ##
###############################################


# config file
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/margot_config.hpp.in"
  "${CONFIG_FILE}"
  )








###############################################
##             pkg-config file               ##
###############################################



# customizing the pkg-config file
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/margot.pc.in"
  "${CMAKE_CURRENT_BINARY_DIR}/margot.pc"
  @ONLY
  )



###############################################
##         Framework Documentation           ##
###############################################


if ( WITH_DOC )

	# set the doxygen output directory
	set( MARGOT_DOC_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc )

	# set the path to the annotated sources
	set( MARGOT_HEADER_ROOT "${MONITOR_HDR_PATH} ${CONFIGURATION_HDR_PATH} ${ASRTM_HDR_PATH}" )

	# set the path to the mainpage
	set( MARGOT_PATH_MAINPAGE ${CMAKE_CURRENT_SOURCE_DIR}/mainpage.dox )

	# configure the file
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
	add_custom_target(doc ALL
		${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
		COMMENT "Generating API documentation with Doxygen" VERBATIM
	)

	install( DIRECTORY ${MARGOT_DOC_OUTPUT_DIRECTORY} DESTINATION . )

endif( WITH_DOC )







###############################################
##         Atom autocomplete file            ##
###############################################

# customizing the .clang_complete file for the Atom editor
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/clang_autocomplete.in"
  "${PROJECT_SOURCE_DIR}/.clang_complete"
  @ONLY
  )

###############################################
##      Atom-linter autocomplete file        ##
###############################################

# customizing the .gcc-flags.json.in file for the Atom editor
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/gcc-flags.json.in"
  "${PROJECT_SOURCE_DIR}/.gcc-flags.json"
  @ONLY
  )



###############################################
##           FindMARGOT cmake file             ##
###############################################


# customizing the FindMARGOT.cmake file for the integration with external applications
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/FindMARGOT.cmake.in"
  "${PROJECT_SOURCE_DIR}/cmake/FindMARGOT.cmake"
  @ONLY
  )
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/FindMARGOT.cmake.in"
  "${PROJECT_SOURCE_DIR}/margot_heel/margot_heel_if/cmake/FindMARGOT.cmake"
  @ONLY
  )


# install the config file
install( FILES ${CONFIG_FILE} DESTINATION include/margot )
install( FILES "${CMAKE_CURRENT_BINARY_DIR}/margot.pc" DESTINATION lib/pkgconfig )
install( FILES "${PROJECT_SOURCE_DIR}/cmake/FindMARGOT.cmake" DESTINATION lib/cmake )
