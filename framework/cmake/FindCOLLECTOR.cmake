# - Check for the presence of COLLECTOR
#
# The following variables are set when COLLECTOR is found:
#  HAVE_COLLECTOR       = Set to true, if all components of COLLECTOR
#                          have been found.
#  COLLECTOR_INCLUDES   = Include path for the header files of COLLECTOR
#  COLLECTOR_LIBRARIES  = Link these to use COLLECTOR
#
# It first search in COLLECTOR_ROOT


set( COLLECTOR_ROOT $ENV{COLLECTOR_ROOT})


## -----------------------------------------------------------------------------
## Check for the header files

find_path (COLLECTOR_INCLUDES collector.h
	PATHS ${COLLECTOR_ROOT}
	NO_DEFAULT_PATH
	)

find_path (COLLECTOR_INCLUDES collector.h)


## -----------------------------------------------------------------------------
## Check for the library

find_library (COLLECTOR_LIBRARY libcollector.a collector
  PATHS ${COLLECTOR_ROOT}
  NO_DEFAULT_PATH
  )

find_library (COLLECTOR_LIBRARY libcollector.a collector)


## -----------------------------------------------------------------------------
## Check for the mosquitto


find_library (COLLECTOR_LIBRARY libmosquitto.a mosquitto
  PATHS ${COLLECTOR_ROOT}/../lib/mosquitto-1.3.5/lib
  NO_DEFAULT_PATH
  )

find_library (COLLECTOR_LIBRARY libcollector.a collector)


## -----------------------------------------------------------------------------
## Check for other dependencies

find_library (SSL_LIBRARY ssl)
find_library (CRYPTO_LIBRARY crypto)
find_library (PTHREAD_LIBRARY pthread)


## -----------------------------------------------------------------------------
## Compose the libraries

set( COLLECTOR_LIBRARIES ${COLLECTOR_LIBRARY} ${MOSQUITTO_LIBRARY} ${SSL_LIBRARY} ${CRYPTO_LIBRARY} ${PTHREAD_LIBRARY})



## -----------------------------------------------------------------------------
## Actions taken when all components have been found

if (COLLECTOR_INCLUDES AND COLLECTOR_LIBRARIES)
  set (HAVE_COLLECTOR TRUE)
else (COLLECTOR_INCLUDES AND COLLECTOR_LIBRARIES)
  if (NOT COLLECTOR_FIND_QUIETLY)
    if (NOT COLLECTOR_INCLUDES)
      message (STATUS "Unable to find COLLECTOR header files!")
    endif (NOT COLLECTOR_INCLUDES)
    if (NOT COLLECTOR_LIBRARIES)
      message (STATUS "Unable to find COLLECTOR library files!")
    endif (NOT COLLECTOR_LIBRARIES)
  endif (NOT COLLECTOR_FIND_QUIETLY)
endif (COLLECTOR_INCLUDES AND COLLECTOR_LIBRARIES)

if (HAVE_COLLECTOR)
  if (NOT COLLECTOR_FIND_QUIETLY)
    message (STATUS "Found components for COLLECTOR")
    message (STATUS "COLLECTOR_INCLUDES .... = ${COLLECTOR_INCLUDES}")
    message (STATUS "COLLECTOR_LIBRARIES ... = ${COLLECTOR_LIBRARIES}")
  endif (NOT COLLECTOR_FIND_QUIETLY)
else (HAVE_COLLECTOR)
  if (COLLECTOR_FIND_REQUIRED)
    message (FATAL_ERROR "Could not find COLLECTOR!")
  endif (COLLECTOR_FIND_REQUIRED)
endif (HAVE_COLLECTOR)

mark_as_advanced (
  HAVE_COLLECTOR
  COLLECTOR_LIBRARIES
  COLLECTOR_INCLUDES
  )