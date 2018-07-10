# - Check for the presence of LIBUV
#
# The following variables are set when LIBUV is found:
#  HAVE_LIBUV       = Set to true, if all components of LIBUV
#                          have been found.
#  LIBUV_INCLUDES   = Include path for the header files of LIBUV
#  LIBUV_LIBRARIES  = Link these to use LIBUV
#
# It first search in LIBUV_ROOT


set( LIBUV_ROOT $ENV{LIBUV_ROOT})


## -----------------------------------------------------------------------------
## Check for the header files

find_path (LIBUV_INCLUDES uv.h uv.h
	PATHS ${LIBUV_ROOT}/include
	NO_DEFAULT_PATH
	)

find_path (LIBUV_INCLUDES uv.h)


## -----------------------------------------------------------------------------
## Check for the library

find_library (LIBUV_LIBRARIES libuv.a uv libuv libuv.so 
  PATHS ${LIBUV_ROOT}/lib ${LIBUV_ROOT}/lib64
  NO_DEFAULT_PATH
  )

find_library (LIBUV_LIBRARIES libuv.a uv libuv libuv.so)


## -----------------------------------------------------------------------------
## Actions taken when all components have been found

if (LIBUV_INCLUDES AND LIBUV_LIBRARIES)
  set (HAVE_LIBUV TRUE)
else (LIBUV_INCLUDES AND LIBUV_LIBRARIES)
  if (NOT LIBUV_FIND_QUIETLY)
    if (NOT LIBUV_INCLUDES)
      message (STATUS "Unable to find LIBUV header files!")
    endif (NOT LIBUV_INCLUDES)
    if (NOT LIBUV_LIBRARIES)
      message (STATUS "Unable to find LIBUV library files!")
    endif (NOT LIBUV_LIBRARIES)
  endif (NOT LIBUV_FIND_QUIETLY)
endif (LIBUV_INCLUDES AND LIBUV_LIBRARIES)

if (HAVE_LIBUV)
  if (NOT LIBUV_FIND_QUIETLY)
    message (STATUS "Found components for LIBUV")
    message (STATUS "LIBUV_INCLUDES .... = ${LIBUV_INCLUDES}")
    message (STATUS "LIBUV_LIBRARIES ... = ${LIBUV_LIBRARIES}")
  endif (NOT LIBUV_FIND_QUIETLY)
else (HAVE_LIBUV)
  if (LIBUV_FIND_REQUIRED)
    message (FATAL_ERROR "Could not find LIBUV!")
  endif (LIBUV_FIND_REQUIRED)
endif (HAVE_LIBUV)

mark_as_advanced (
  HAVE_LIBUV
  LIBUV_LIBRARIES
  LIBUV_INCLUDES
  )
