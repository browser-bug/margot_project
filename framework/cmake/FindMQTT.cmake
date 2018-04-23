# - Check for the presence of MQTT
#
# The following variables are set when MQTT is found:
#  HAVE_MQTT       = Set to true, if all components of MQTT
#                          have been found.
#  MQTT_INCLUDES   = Include path for the header files of MQTT
#  MQTT_LIBRARIES  = Link these to use MQTT
#
# It first search in MQTT_ROOT


set( MQTT_ROOT $ENV{MQTT_ROOT})


## -----------------------------------------------------------------------------
## Check for the header files

find_path (MQTT_INCLUDES MQTTClient.h
  PATHS ${MQTT_ROOT}/include ${PROJECT_SOURCE_DIR}/extern/install/include
  NO_DEFAULT_PATH
  )

find_path (MQTT_INCLUDES MQTTClient.h)


## -----------------------------------------------------------------------------
## Check for the paho mqtt library

find_library (MQTT_LIBRARY libpaho-mqtt3cs-static.a paho-mqtt3cs
  PATHS ${MQTT_ROOT}/lib ${MQTT_ROOT}/lib64 ${PROJECT_SOURCE_DIR}/extern/install/lib ${PROJECT_SOURCE_DIR}/extern/install/lib64
  NO_DEFAULT_PATH
  )

find_library (MQTT_LIBRARY libpaho-mqtt3cs-static.a paho-mqtt3cs)


## -----------------------------------------------------------------------------
## Check for the pthread library

find_library (MQTT_PTHREADS pthread)

## -----------------------------------------------------------------------------
## Compose the libraries

set( MQTT_LIBRARIES ${MQTT_LIBRARY} ${MQTT_PTHREADS} )



## -----------------------------------------------------------------------------
## Actions taken when all components have been found

if (MQTT_INCLUDES AND MQTT_LIBRARIES)
  set (HAVE_MQTT TRUE)
else (MQTT_INCLUDES AND MQTT_LIBRARIES)
  if (NOT MQTT_FIND_QUIETLY)
    if (NOT MQTT_INCLUDES)
      message (STATUS "Unable to find MQTT header files!")
    endif (NOT MQTT_INCLUDES)
    if (NOT MQTT_LIBRARIES)
      message (STATUS "Unable to find MQTT library files!")
      message (STATUS "  Libraries: ${MQTT_LIBRARIES}")
    endif (NOT MQTT_LIBRARIES)
  endif (NOT MQTT_FIND_QUIETLY)
endif (MQTT_INCLUDES AND MQTT_LIBRARIES)

if (HAVE_MQTT)
  if (NOT MQTT_FIND_QUIETLY)
    message (STATUS "Found components for MQTT")
    message (STATUS "MQTT_INCLUDES .... = ${MQTT_INCLUDES}")
    message (STATUS "MQTT_LIBRARIES ... = ${MQTT_LIBRARIES}")
  endif (NOT MQTT_FIND_QUIETLY)
endif (HAVE_MQTT)

mark_as_advanced (
  HAVE_MQTT
  MQTT_LIBRARIES
  MQTT_INCLUDES
  )
