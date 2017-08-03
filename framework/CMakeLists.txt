################################
#### Compile framework library
################################

set( HDR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include" )
set( MARGOT_HEADERS
	${HDR_PATH}/margot/basic_information_block.hpp
	${HDR_PATH}/margot/hash.hpp
	${HDR_PATH}/margot/operating_point.hpp
	${HDR_PATH}/margot/operating_point_segment.hpp
	${HDR_PATH}/margot/traits.hpp
	)


include_directories( ${HDR_PATH} )


install( FILES ${MARGOT_HEADERS} DESTINATION include/margot )




################################
#### Build additional stuff
################################

if ( WITH_TEST )
	add_subdirectory( test )
endif ( WITH_TEST )

if ( GEN_DOC )
	add_subdirectory( doc )
endif ( GEN_DOC )
