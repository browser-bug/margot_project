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
	${HDR_PATH}/margot/knowledge_base.hpp
	${HDR_PATH}/margot/circular_buffer.hpp
	${HDR_PATH}/margot/statistical_provider.hpp
	${HDR_PATH}/margot/statistics.hpp
	${HDR_PATH}/margot/enmus.hpp
	${HDR_PATH}/margot/field_adaptor.hpp
	${HDR_PATH}/margot/knowledge_adaptor.hpp
	${HDR_PATH}/margot/evaluator.hpp
	${HDR_PATH}/margot/view.hpp
	${HDR_PATH}/margot/monitor.hpp
	${HDR_PATH}/margot/rank.hpp
	${HDR_PATH}/margot/goal.hpp
	${HDR_PATH}/margot/constraint.hpp
	${HDR_PATH}/margot/state.hpp
	${HDR_PATH}/margot/asrtm.hpp
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
