# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindCOLLECTOR
-------

Finds the COLLECTOR library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Collector::Collector``
  The COLLECTOR library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``COLLECTOR_FOUND``
  True if the system has the COLLECTOR library.
``COLLECTOR_VERSION``
  The version of the COLLECTOR library which was found.
``COLLECTOR_INCLUDE_DIRS``
  Include directories needed to use COLLECTOR.
``COLLECTOR_LIBRARIES``
  Libraries needed to link to COLLECTOR.

Hints
^^^^^

It is possible to suggest the directory where COLLECTOR is located by
setting the variable ``COLLECTOR_ROOT_DIR``

#]=======================================================================]


# Look for the headers and libraries of the package
find_path(COLLECTOR_INCLUDE_DIRS
  NAMES collector.h
  PATHS ${COLLECTOR_ROOT_DIR}/include ${COLLECTOR_ROOT_DIR}
  SUFFIX COLLECTOR
)
find_library(COLLECTOR_LIBRARIES
  NAMES libcollector.a collector
  PATHS ${COLLECTOR_ROOT_DIR}/lib ${COLLECTOR_ROOT_DIR}
)


# Now we can let cmake to handle the standard variables
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(COLLECTOR
  FOUND_VAR COLLECTOR_FOUND
  REQUIRED_VARS
    COLLECTOR_LIBRARIES
    COLLECTOR_INCLUDE_DIRS
)

# And import the target to provide the new way of compilation
if(COLLECTOR_FOUND AND NOT TARGET Collector::Collector)
  add_library(Collector::Collector UNKNOWN IMPORTED)
  set_target_properties(Collector::Collector PROPERTIES
    IMPORTED_LOCATION "${COLLECTOR_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${COLLECTOR_INCLUDE_DIRS}"
  )
  add_dependencies( Collector::Collector PahoMQTT::PahoMQTT OpenSSL::SSL OpenSSL::Crypto )
endif()


# Now we can avoid to set the header and library in the cache
mark_as_advanced(
  COLLECTOR_INCLUDE_DIRS
  COLLECTOR_LIBRARIES
)
