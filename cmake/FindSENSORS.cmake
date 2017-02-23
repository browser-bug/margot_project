# - Check for the presence of SENSORS
#
# The following variables are set when SENSORS is found:
#  HAVE_SENSORS       = Set to true, if all components of SENSORS
#                          have been found.
#  SENSORS_INCLUDES   = Include path for the header files of SENSORS
#  SENSORS_LIBRARIES  = Link these to use SENSORS
#
# It first search in SENSORS_ROOT

set( SENSORS_ROOT $ENV{SENSORS_ROOT})

## -----------------------------------------------------------------------------
## Check for the header files

find_path (SENSORS_INCLUDES sensors/sensors.h
  PATHS ${SENSORS_ROOT}/include
  NO_DEFAULT_PATH
  )

if (NOT SENSORS_INCLUDES)
  find_path (SENSORS_INCLUDES sensors/sensors.h
    PATHS /usr/local /usr ${CMAKE_EXTRA_INCLUDES}
    )
endif(NOT SENSORS_INCLUDES)

## -----------------------------------------------------------------------------
## Check for the library

find_library (SENSORS_LIBRARIES libsensors.so.4 libsensors.so sensors
  PATHS ${SENSORS_ROOT}
  NO_DEFAULT_PATH
  )

if (NOT SENSORS_LIBRARIES)
  find_library (SENSORS_LIBRARIES libsensors.so.4 libsensors.so sensors
    PATHS /usr/local/lib /usr/lib /lib ${CMAKE_EXTRA_LIBRARIES}
    )
endif (NOT SENSORS_LIBRARIES)

## -----------------------------------------------------------------------------
## Actions taken when all components have been found

if (SENSORS_INCLUDES AND SENSORS_LIBRARIES)
  set (HAVE_SENSORS TRUE)
else (SENSORS_INCLUDES AND SENSORS_LIBRARIES)
  if (NOT SENSORS_FIND_QUIETLY)
    if (NOT SENSORS_INCLUDES)
      message (STATUS "Unable to find SENSORS header files!")
    endif (NOT SENSORS_INCLUDES)
    if (NOT SENSORS_LIBRARIES)
      message (STATUS "Unable to find SENSORS library files!")
    endif (NOT SENSORS_LIBRARIES)
  endif (NOT SENSORS_FIND_QUIETLY)
endif (SENSORS_INCLUDES AND SENSORS_LIBRARIES)

if (HAVE_SENSORS)
  if (NOT SENSORS_FIND_QUIETLY)
    message (STATUS "Found components for SENSORS")
    message (STATUS "SENSORS_INCLUDES ... = ${SENSORS_INCLUDES}")
    message (STATUS "SENSORS_LIBRARIES .. = ${SENSORS_LIBRARIES}")
  endif (NOT SENSORS_FIND_QUIETLY)
else (HAVE_SENSORS)
  if (SENSORS_FIND_REQUIRED)
    message (FATAL_ERROR "Could not find SENSORS!")
  endif (SENSORS_FIND_REQUIRED)
endif (HAVE_SENSORS)

mark_as_advanced (
  HAVE_SENSORS
  SENSORS_LIBRARIES
  SENSORS_INCLUDES
  )
