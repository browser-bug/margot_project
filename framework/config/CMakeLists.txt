###############################################
##             pkg-config file               ##
###############################################

# replace the ; character with space for link flangs
string (REPLACE " " "\\ " MARGOT_LINK_DEPENDENCIES_STR_TEMP "${MARGOT_LINK_DEPENDENCIES}")
string (REPLACE ";" " " MARGOT_LINK_DEPENDENCIES_STR "${MARGOT_LINK_DEPENDENCIES_STR_TEMP}")
string (REPLACE " " "\\ " MARGOT_DEPS_INCLUDE_DIRECTORIES_STR_TEMP "${MARGOT_DEPS_INCLUDE_DIRECTORIES}")
string (REPLACE ";" " -I" MARGOT_DEPS_INCLUDE_DIRECTORIES_STR "${MARGOT_DEPS_INCLUDE_DIRECTORIES_STR_TEMP}")

# customizing the pkg-config file
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/margot.pc.in"
  "${PROJECT_BINARY_DIR}/finders/margot.pc"
  @ONLY
  )

install( FILES "${PROJECT_BINARY_DIR}/finders/margot.pc" DESTINATION lib/pkgconfig )



###############################################
##           FindMARGOT cmake file           ##
###############################################


# customizing the FindMARGOT.cmake file for the integration with external applications
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/FindMARGOT.cmake.in"
  "${PROJECT_BINARY_DIR}/finders/FindMARGOT.cmake"
  @ONLY
  )

# store a copy also in the margot heel cmake folder
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/FindMARGOT.cmake.in"
  "${PROJECT_SOURCE_DIR}/margot_heel/margot_heel_if/cmake/FindMARGOT.cmake"
  @ONLY
  )

install( FILES "${PROJECT_BINARY_DIR}/finders/FindMARGOT.cmake" DESTINATION lib/cmake )


###############################################
##             Configure margot              ##
###############################################

if ( WITH_AGORA )
  set( MARGOT_WITH_AGORA ON )
endif ( WITH_AGORA )
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/margot_config.hpp.in"
  "${MARGOT_CONFIG_FILE_PATH}/margot/margot_config.hpp"
  )
