# - Check for the presence of PAPI
#
# The following variables are set when PAPI is found:
#  HAVE_PAPI       = Set to true, if all components of PAPI
#                          have been found.
#  PAPI_INCLUDES   = Include path for the header files of PAPI
#  PAPI_LIBRARIES  = Link these to use PAPI
#
# It first search in PAPI_ROOT

set( PAPI_ROOT $ENV{PAPI_ROOT})

## -----------------------------------------------------------------------------
## Check for the header files

find_path (PAPI_INCLUDES papi.h
  PATHS ${PAPI_ROOT}/include
  NO_DEFAULT_PATH
  )

if (NOT PAPI_INCLUDES)
  find_path (PAPI_INCLUDES include/papi.h
    PATHS /usr/local /usr ${CMAKE_EXTRA_INCLUDES}
    )
endif(NOT PAPI_INCLUDES)

## -----------------------------------------------------------------------------
## Check for the library

find_library (PAPI_LIBRARIES libpapi.a papi
  PATHS ${PAPI_ROOT}/lib
  NO_DEFAULT_PATH
  )

if (NOT PAPI_LIBRARIES)
  find_library (PAPI_LIBRARIES libpapi.a papi
    PATHS /usr/local/lib /usr/lib /lib ${CMAKE_EXTRA_LIBRARIES}
    )
endif (NOT PAPI_LIBRARIES)

set(PAPI_LIBRARIES ${PAPI_LIBRARIES} pfm)

## -----------------------------------------------------------------------------
## Actions taken when all components have been found

if (PAPI_INCLUDES AND PAPI_LIBRARIES)
  set (HAVE_PAPI TRUE)
else (PAPI_INCLUDES AND PAPI_LIBRARIES)
  if (NOT PAPI_FIND_QUIETLY)
    if (NOT PAPI_INCLUDES)
      message (STATUS "Unable to find PAPI header files!")
    endif (NOT PAPI_INCLUDES)
    if (NOT PAPI_LIBRARIES)
      message (STATUS "Unable to find PAPI library files!")
    endif (NOT PAPI_LIBRARIES)
  endif (NOT PAPI_FIND_QUIETLY)
endif (PAPI_INCLUDES AND PAPI_LIBRARIES)

if (HAVE_PAPI)
  if (NOT PAPI_FIND_QUIETLY)
    message (STATUS "Found components for PAPI")
    message (STATUS "PAPI_INCLUDES .... = ${PAPI_INCLUDES}")
    message (STATUS "PAPI_LIBRARIES ... = ${PAPI_LIBRARIES}")
  endif (NOT PAPI_FIND_QUIETLY)
else (HAVE_PAPI)
  if (PAPI_FIND_REQUIRED)
    message (FATAL_ERROR "Could not find PAPI!")
  endif (PAPI_FIND_REQUIRED)
endif (HAVE_PAPI)

mark_as_advanced (
  HAVE_PAPI
  PAPI_LIBRARIES
  PAPI_INCLUDES
  )
