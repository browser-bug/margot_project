# - Check for the presence of CASSANDRA
#
# The following variables are set when CASSANDRA is found:
#  HAVE_CASSANDRA       = Set to true, if all components of CASSANDRA
#                          have been found.
#  CASSANDRA_INCLUDES   = Include path for the header files of CASSANDRA
#  CASSANDRA_LIBRARIES  = Link these to use CASSANDRA
#
# It first search in CASSANDRA_ROOT


set( CASSANDRA_ROOT $ENV{CASSANDRA_ROOT})


## -----------------------------------------------------------------------------
## Check for the header files

find_path (CASSANDRA_INCLUDES cassandra.h
  PATHS ${CASSANDRA_ROOT}/include ${PROJECT_SOURCE_DIR}/extern/install/include
  NO_DEFAULT_PATH
  )

find_path (CASSANDRA_INCLUDES cassandra.h)


## -----------------------------------------------------------------------------
## Check for the DataStax driver library

find_library (CASSANDRA_LIBRARY libcassandra_static.a cassandra_static libcassandra.so cassandra
  PATHS ${CASSANDRA_ROOT}/lib ${CASSANDRA_ROOT}/lib64 ${PROJECT_SOURCE_DIR}/extern/install/lib ${PROJECT_SOURCE_DIR}/extern/install/lib64
  NO_DEFAULT_PATH
  )

find_library (CASSANDRA_LIBRARY libcassandra_static.a cassandra_static libcassandra.so cassandra)


## -----------------------------------------------------------------------------
## Check for the libvu library

find_package ( LIBUV )


## -----------------------------------------------------------------------------
## Check for the openssl library

find_package ( OpenSSL )


## -----------------------------------------------------------------------------
## Compose the libraries

set ( CASSANDRA_LIBRARIES ${CASSANDRA_LIBRARY} ${LIBUV_LIBRARIES} ${OPENSSL_LIBRARIES} )


## -----------------------------------------------------------------------------
## Actions taken when all components have been found

if (CASSANDRA_INCLUDES AND CASSANDRA_LIBRARIES)
  set (HAVE_CASSANDRA TRUE)
else (CASSANDRA_INCLUDES AND CASSANDRA_LIBRARIES)
  if (NOT CASSANDRA_FIND_QUIETLY)
    if (NOT CASSANDRA_INCLUDES)
      message (STATUS "Unable to find CASSANDRA header files!")
    endif (NOT CASSANDRA_INCLUDES)
    if (NOT CASSANDRA_LIBRARIES)
      message (STATUS "Unable to find CASSANDRA library files!")
      message (STATUS "  Libraries: ${CASSANDRA_LIBRARIES}")
    endif (NOT CASSANDRA_LIBRARIES)
  endif (NOT CASSANDRA_FIND_QUIETLY)
endif (CASSANDRA_INCLUDES AND CASSANDRA_LIBRARIES)

if (HAVE_CASSANDRA)
  if (NOT CASSANDRA_FIND_QUIETLY)
    message (STATUS "Found components for CASSANDRA")
    message (STATUS "CASSANDRA_INCLUDES .... = ${CASSANDRA_INCLUDES}")
    message (STATUS "CASSANDRA_LIBRARIES ... = ${CASSANDRA_LIBRARIES}")
  endif (NOT CASSANDRA_FIND_QUIETLY)
endif (HAVE_CASSANDRA)

mark_as_advanced (
  HAVE_CASSANDRA
  CASSANDRA_LIBRARIES
  CASSANDRA_INCLUDES
  )
