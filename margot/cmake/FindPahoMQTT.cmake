# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindPahoMQTT
-------

Finds the PahoMQTT library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``PahoMQTT::PahoMQTT``
  The PahoMQTT library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``PAHOMQTT_FOUND``
  True if the system has the PahoMQTT library.
``PAHOMQTT_VERSION``
  The version of the PahoMQTT library which was found.
``PAHOMQTT_INCLUDE_DIRS``
  Include directories needed to use PahoMQTT.
``PAHOMQTT_LIBRARIES``
  Libraries needed to link to PahoMQTT.

Hints
^^^^^

It is possible to suggest the directory where PahoMQTT is located by
setting the variable ``PAHOMQTT_ROOT_DIR``

#]=======================================================================]


# Look for the headers and libraries of the package
find_path(PAHOMQTT_INCLUDE_DIRS
  NAMES MQTTClient.h
  PATHS ${PAHOMQTT_ROOT_DIR}/include ${PAHOMQTT_ROOT_DIR}
)
find_library(PAHOMQTT_LIBRARIES
  NAMES libpaho-mqtt3cs-static.a paho-mqtt3cs
  PATHS ${PAHOMQTT_ROOT_DIR}/lib ${PAHOMQTT_ROOT_DIR}
)


# Now we can let cmake to handle the standard variables
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PahoMQTT
  FOUND_VAR PAHOMQTT_FOUND
  REQUIRED_VARS
    PAHOMQTT_LIBRARIES
    PAHOMQTT_INCLUDE_DIRS
)

# And import the target to provide the new way of compilation
if(PahoMQTT_FOUND AND NOT TARGET PahoMQTT::PahoMQTT)
  add_library(PahoMQTT::PahoMQTT UNKNOWN IMPORTED)
  set_target_properties(PahoMQTT::PahoMQTT PROPERTIES
    IMPORTED_LOCATION "${PAHOMQTT_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${PAHOMQTT_INCLUDE_DIRS}"
  )
  add_dependencies( PahoMQTT::PahoMQTT OpenSSL::SSL OpenSSL::Crypto )
endif()


# Now we can avoid to set the header and library in the cache
mark_as_advanced(
  PAHOMQTT_INCLUDE_DIRS
  PAHOMQTT_LIBRARIES
)



