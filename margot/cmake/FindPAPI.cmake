# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindPAPI
-------

Finds the PAPI library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Papi::Papi``
  The PAPI library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``PAPI_FOUND``
  True if the system has the PAPI library.
``PAPI_VERSION``
  The version of the PAPI library which was found.
``PAPI_INCLUDE_DIRS``
  Include directories needed to use PAPI.
``PAPI_LIBRARIES``
  Libraries needed to link to PAPI.

Hints
^^^^^

It is possible to suggest the directory where PAPI is located by
setting the variable ``PAPI_ROOT_DIR``

#]=======================================================================]


# Look for the headers and libraries of the package
find_path(PAPI_INCLUDE_DIRS
  NAMES papi.h
  PATHS ${PAPI_ROOT_DIR}/include ${PAPI_ROOT_DIR}
)
find_library(PAPI_LIBRARIES
  NAMES libpapi.a papi
  PATHS ${PAPI_ROOT_DIR}/lib ${PAPI_ROOT_DIR}
)


# Now we can let cmake to handle the standard variables
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PAPI
  FOUND_VAR PAPI_FOUND
  REQUIRED_VARS
    PAPI_LIBRARIES
    PAPI_INCLUDE_DIRS
)

# And import the target to provide the new way of compilation
if(PAPI_FOUND AND NOT TARGET Papi::Papi)
  add_library(Papi::Papi UNKNOWN IMPORTED)
  set_target_properties(Papi::Papi PROPERTIES
    IMPORTED_LOCATION "${PAPI_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${PAPI_INCLUDE_DIRS}"
  )
  add_dependencies( Papi::Papi Pfm:Pfm )
endif()


# Now we can avoid to set the header and library in the cache
mark_as_advanced(
  PAPI_INCLUDE_DIRS
  PAPI_LIBRARIES
)
