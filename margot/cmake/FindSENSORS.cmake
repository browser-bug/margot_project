# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindSENSORS
-------

Finds the SENSORS library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Sensors::Sensors``
  The SENSORS library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``SENSORS_FOUND``
  True if the system has the SENSORS library.
``SENSORS_VERSION``
  The version of the SENSORS library which was found.
``SENSORS_INCLUDE_DIRS``
  Include directories needed to use SENSORS.
``SENSORS_LIBRARIES``
  Libraries needed to link to SENSORS.

Hints
^^^^^

It is possible to suggest the directory where SENSORS is located by
setting the variable ``SENSORS_ROOT_DIR``

#]=======================================================================]


# Look for the headers and libraries of the package
find_path(SENSORS_INCLUDE_DIRS
  NAMES sensors.h
  PATHS ${SENSORS_ROOT_DIR}/include ${SENSORS_ROOT_DIR}
  SUFFIX sensors
)
find_library(SENSORS_LIBRARIES
  NAMES libsensors.so.4 libsensors.so sensors
  PATHS ${SENSORS_ROOT_DIR}/lib ${SENSORS_ROOT_DIR}
)


# Now we can let cmake to handle the standard variables
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SENSORS
  FOUND_VAR SENSORS_FOUND
  REQUIRED_VARS
    SENSORS_LIBRARIES
    SENSORS_INCLUDE_DIRS
)

# And import the target to provide the new way of compilation
if(SENSORS_FOUND AND NOT TARGET Sensors::Sensors)
  add_library(Sensors::Sensors UNKNOWN IMPORTED)
  set_target_properties(Sensors::Sensors PROPERTIES
    IMPORTED_LOCATION "${SENSORS_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${SENSORS_INCLUDE_DIRS}"
  )
endif()


# Now we can avoid to set the header and library in the cache
mark_as_advanced(
  SENSORS_INCLUDE_DIRS
  SENSORS_LIBRARIES
)

