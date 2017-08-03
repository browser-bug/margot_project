# - Check for the presence of CXXTEST
#
# The following variables are set when CXXTEST is found:
#  HAVE_CXXTEST       = Set to true, if all components of CXXTEST
#                          have been found.
#  CXXTEST_INCLUDES   = Include path for the header files of CXXTEST
#  CXXTEST_BIN  = Link these to use CXXTEST
#
# It first search in CXXTEST_ROOT

set( CXXTEST_ROOT $ENV{CXXTEST_ROOT})

## -----------------------------------------------------------------------------
## Check for the header files

find_path (CXXTEST_INCLUDES cxxtest/TestSuite.h
	PATHS ${CXXTEST_ROOT} ${PROJECT_SOURCE_DIR}/cxxtest
	NO_DEFAULT_PATH
	)

find_path (CXXTEST_INCLUDES cxxtest/TestSuite.h)

## -----------------------------------------------------------------------------
## Check for the executable

find_program (CXXTEST_BIN
	NAMES cxxtestgen
	PATHS ${CXXTEST_ROOT}/bin ${PROJECT_SOURCE_DIR}/cxxtest/bin
	NO_DEFAULT_PATH
	)


find_program (CXXTEST_BIN
	NAMES cxxtestgen
	)


## -----------------------------------------------------------------------------
## Actions taken when all components have been found

if (CXXTEST_INCLUDES AND CXXTEST_BIN)
  set (HAVE_CXXTEST TRUE)
else (CXXTEST_INCLUDES AND CXXTEST_BIN)
  if (NOT CXXTEST_FIND_QUIETLY)
    if (NOT CXXTEST_INCLUDES)
      message (STATUS "Unable to find CXXTEST header files!")
    endif (NOT CXXTEST_INCLUDES)
    if (NOT CXXTEST_BIN)
      message (STATUS "Unable to find CXXTEST binary file!")
    endif (NOT CXXTEST_BIN)
  endif (NOT CXXTEST_FIND_QUIETLY)
endif (CXXTEST_INCLUDES AND CXXTEST_BIN)

if (HAVE_CXXTEST)
  if (NOT CXXTEST_FIND_QUIETLY)
    message (STATUS "Found components for CXXTEST")
    message (STATUS "CXXTEST_INCLUDES .... = ${CXXTEST_INCLUDES}")
    message (STATUS "CXXTEST_BIN ... = ${CXXTEST_BIN}")
  endif (NOT CXXTEST_FIND_QUIETLY)
else (HAVE_CXXTEST)
  if (CXXTEST_FIND_REQUIRED)
    message (FATAL_ERROR "Could not find CXXTEST!")
  endif (CXXTEST_FIND_REQUIRED)
endif (HAVE_CXXTEST)

mark_as_advanced (
  HAVE_CXXTEST
  CXXTEST_BIN
  CXXTEST_INCLUDES
  )
