# - Check for the presence of the Argo framework
#
# The following variables are set when Argo is found:
#  HAVE_MARGOT         = Set to true, if all components of Argo
#                          have been found.
#  MARGOT_INCLUDES  = Include path for the header files of Argo
#  MARGOT_LIBRARIES    = Link these to use ANTAREX
#  MARGOT_CLI_COMMAND  = Path to the command line interface binary
#

## -----------------------------------------------------------------------------
## Project wide configurations path and informations

set( MARGOT_SOURCE_PATH /home/dgadioli/Projects/margot_project/core/margot )
set( MARGOT_BINARY_PATH /home/dgadioli/Projects/margot_project/core/margot/build )
set( MARGOT_INSTALL_PATH /home/dgadioli/Projects/margot_project/core/margot )
set( MARGOT_DEPS_HEADERS  )
set( MARGOT_DEPS_LIBS  )


## -----------------------------------------------------------------------------
## Check for the header files

# check for the header files
find_path (MARGOT_HEADERS margot/monitor.hpp
  PATHS ${MARGOT_INSTALL_PATH}/include ${MARGOT_SOURCE_PATH}/framework/include
  NO_DEFAULT_PATH
  )
find_path (MARGOT_HEADERS margot/monitor.hpp)


## -----------------------------------------------------------------------------
## Check for the configuration header file

find_path (MARGOT_CONF_HEADER margot/margot_config.hpp
  PATHS ${MARGOT_INSTALL_PATH}/include ${MARGOT_BINARY_PATH}/include
  NO_DEFAULT_PATH
  )
find_path (MARGOT_CONF_HEADER margot/margot_config.hpp)


# add the paths for the dependecies
set( MARGOT_INCLUDES ${MARGOT_HEADERS} ${MARGOT_CONF_HEADER} ${MARGOT_DEPS_HEADERS} )


## -----------------------------------------------------------------------------
## Check for the libraries

find_library (MARGOT_LIBRARY libmargot.a libmargot.so margot
  PATHS ${MARGOT_INSTALL_PATH}/lib ${MARGOT_BINARY_PATH}/framework
  NO_DEFAULT_PATH
  )

find_library (SENSORS_LIBRARY libmargot.a libmargot.so margot)

set( MARGOT_LIBRARIES ${MARGOT_LIBRARY} ${MARGOT_DEPS_LIBS} )


## -----------------------------------------------------------------------------
## Check for the margot cli executable

find_program (MARGOT_CLI_COMMAND
	NAMES margot_cli
	PATHS ${MARGOT_INSTALL_PATH}/bin ${MARGOT_SOURCE_PATH}/margot_heel/margot_heel_cli/bin
	NO_DEFAULT_PATH
	)


find_program (MARGOT_CLI_COMMAND
	NAMES margot_cli
	)


## -----------------------------------------------------------------------------
## Actions taken when all components have been found


if (MARGOT_INCLUDES AND MARGOT_LIBRARIES AND MARGOT_CLI_COMMAND)
  set (HAVE_MARGOT TRUE)
else (MARGOT_INCLUDES AND MARGOT_LIBRARIES AND MARGOT_CLI_COMMAND)
  if (NOT MARGOT_FIND_QUIETLY)
    if (NOT (MARGOT_INCLUDES))
      message (STATUS "Unable to find MARGOT header files!")
    endif (NOT (MARGOT_INCLUDES))
    if (NOT (MARGOT_LIBRARIES))
      message (STATUS "Unable to find MARGOT library files!")
    endif (NOT (MARGOT_LIBRARIES))
    if (NOT MARGOT_CLI_COMMAND)
      message (STATUS "Unable to fine MARGOT heel command line interface")
   endif (NOT MARGOT_CLI_COMMAND)
  endif (NOT MARGOT_FIND_QUIETLY)
endif (MARGOT_INCLUDES AND MARGOT_LIBRARIES AND MARGOT_CLI_COMMAND)

if (HAVE_MARGOT)
  if (NOT MARGOT_FIND_QUIETLY)
    message (STATUS "Found components for MARGOT")
    message (STATUS "MARGOT_INCLUDES .... = ${MARGOT_INCLUDES}")
    message (STATUS "MARGOT_LIBRARIES ... = ${MARGOT_LIBRARIES}")
    message (STATUS "MARGOT_CLI ......... = ${MARGOT_CLI_COMMAND}")
  endif (NOT MARGOT_FIND_QUIETLY)
else (HAVE_MARGOT)
  if (MARGOT_FIND_QUIETLY)
    message (FATAL_ERROR "Could not find MARGOT!")
endif (MARGOT_FIND_QUIETLY)
endif (HAVE_MARGOT)

mark_as_advanced (
  HAVE_MARGOT
  MARGOT_INCLUDES
  MARGOT_LIBRARIES
  MARGOT_CLI_COMMAND
  )