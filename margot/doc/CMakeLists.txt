
################################
#### Configuring doxygen
################################

# set the configuration variables for the configuration file
set( MY_DOXYGEN_PROJECT_TILE "${CMAKE_PROJECT_NAME} Project")
set( MY_DOXYGEN_VERSION "v${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
set( MY_DOXYGEN_OUTPUT_FOLDER "${CMAKE_CURRENT_BINARY_DIR}/doc")
set( MY_DOXYGEN_SOURCE_FOLDER "${MARGOT_HDR_PATH}/margot ${CMAKE_CURRENT_SOURCE_DIR}/mainpage.dox")
#set( MY_DOXYGEN_SOURCE_FOLDER "${HDR_PATH}/margot")

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


add_custom_target(doc ALL
	${DOXYGEN_EXECUTABLE} ${MY_DOXYGEN_CONF_FILE_PATH}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMENT "Generating API documentation with Doxygen" VERBATIM
)


install( DIRECTORY ${MY_DOXYGEN_OUTPUT_FOLDER} DESTINATION . )
