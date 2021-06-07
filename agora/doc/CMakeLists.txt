################################
#### Configuring doxygen
################################

# set the configuration variables for the configuration file
set( MY_DOXYGEN_PROJECT_TILE "${CMAKE_PROJECT_NAME} Project")
set( MY_DOXYGEN_VERSION "v${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
set( MY_DOXYGEN_OUTPUT_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/doc")
set( MY_DOXYGEN_SOURCE_FOLDER "${AGORA_HDR_PATH}/agora ${CMAKE_CURRENT_SOURCE_DIR}/mainpage.dox")

# set the path for working with doxygen
set( MY_DOXYGEN_CONF_FILE_PATH "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile")

# generate the actual doxygen file
configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/doxygen.conf.in"
	"${MY_DOXYGEN_CONF_FILE_PATH}"
	@ONLY
	)


################################
#### Generate the documentation
################################


add_custom_target(agora_doc ALL
	${DOXYGEN_EXECUTABLE} ${MY_DOXYGEN_CONF_FILE_PATH}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generating Agora API documentation with Doxygen" VERBATIM
)


# copy the content *inside* MY_DOXYGEN_OUTPUT_FOLDER into destination
install( DIRECTORY ${MY_DOXYGEN_OUTPUT_FOLDER}/ DESTINATION ./doc/agora/ )
