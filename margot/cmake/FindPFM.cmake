# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindPFM
-------

Finds the PFM library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``Pfm:Pfm``
  The PFM library

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``PFM_FOUND``
  True if the system has the PFM library.
``PFM_VERSION``
  The version of the PFM library which was found.
``PFM_INCLUDE_DIRS``
  Include directories needed to use PFM.
``PFM_LIBRARIES``
  Libraries needed to link to PFM.

Hints
^^^^^

It is possible to suggest the directory where PFM is located by
setting the variable ``PFM_ROOT_DIR``

#]=======================================================================]


# Look for the headers and libraries of the package
find_path(PFM_INCLUDE_DIRS
  NAMES pfmlib.h
  PATHS ${PFM_ROOT_DIR}/include ${PFM_ROOT_DIR}
  SUFFIX perfmon
)
find_library(PFM_LIBRARIES
  NAMES libpfm.a pfm
  PATHS ${PFM_ROOT_DIR}/lib ${PFM_ROOT_DIR}
)


# Now we can let cmake to handle the standard variables
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(PFM
  FOUND_VAR PFM_FOUND
  REQUIRED_VARS
    PFM_LIBRARIES
    PFM_INCLUDE_DIRS
)

# And import the target to provide the new way of compilation
if(PFM_FOUND AND NOT TARGET Pfm:Pfm)
  add_library(Pfm:Pfm UNKNOWN IMPORTED)
  set_target_properties(Pfm:Pfm PROPERTIES
    IMPORTED_LOCATION "${PFM_LIBRARIES}"
    INTERFACE_INCLUDE_DIRECTORIES "${PFM_INCLUDE_DIRS}"
  )
endif()


# Now we can avoid to set the header and library in the cache
mark_as_advanced(
  PFM_INCLUDE_DIRS
  PFM_LIBRARIES
)



