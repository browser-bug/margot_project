###############################################
##      Compiling the framework module       ##
###############################################



# -------- Defining the source/header files
set( ASRTM_SRC_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src )


set( ASRTM_HEADERS  ${ASRTM_HDR_PATH}/margot/operating_point.hpp
                    ${ASRTM_HDR_PATH}/margot/view.hpp
                    ${ASRTM_HDR_PATH}/margot/state.hpp
                    ${ASRTM_HDR_PATH}/margot/knowledge_base.hpp
                    ${ASRTM_HDR_PATH}/margot/rank_calculator.hpp
                    ${ASRTM_HDR_PATH}/margot/asrtm.hpp
                    ${ASRTM_HDR_PATH}/margot/learning_state.hpp
                    ${ASRTM_HDR_PATH}/margot/mab_sw_ucb.hpp
                    ${CONFIG_FILE}
   )
set( ASRTM_SOURCES  ${ASRTM_SRC_PATH}/view
                    ${ASRTM_SRC_PATH}/state
                    ${ASRTM_SRC_PATH}/knowledge_base
                    ${ASRTM_SRC_PATH}/rank_calculator
                    ${ASRTM_SRC_PATH}/asrtm
                    ${ASRTM_SRC_PATH}/mab_sw_ucb
   )



# --------  Build the module
include_directories(${MONITOR_HDR_PATH})
include_directories(${ASRTM_HDR_PATH})
include_directories(${CONFIGURATION_HDR_PATH})



if   ( LIB_STATIC )
        add_library( ${ASRTM_LIB_NAME} STATIC ${ASRTM_SOURCES} ${ASRTM_HEADERS} )
else ( LIB_STATIC )
        add_library( ${ASRTM_LIB_NAME} SHARED ${ASRTM_SOURCES} ${ASRTM_HEADERS} )
endif( LIB_STATIC )

target_link_libraries( ${ASRTM_LIB_NAME} ${MONITOR_LIB_NAME})
if( HAVE_PAPI )
        target_link_libraries(${ASRTM_LIB_NAME} ${PAPI_LIBRARIES})
endif( HAVE_PAPI )
if( HAVE_SENSORS )
        target_link_libraries(${ASRTM_LIB_NAME} ${SENSORS_LIBRARIES})
endif( HAVE_SENSORS )



# ------ Install the module
install( TARGETS ${ASRTM_LIB_NAME} DESTINATION lib )
install( FILES ${ASRTM_HEADERS} DESTINATION include/margot )




###############################################
##      Adding the headers to the test       ##
###############################################

# this variable it is used to build the test suite
set ( ASRTM_TEST_HEADERS_TEMP ${CMAKE_CURRENT_SOURCE_DIR}/test/test_op.hpp
                              ${CMAKE_CURRENT_SOURCE_DIR}/test/test_view.hpp
                              ${CMAKE_CURRENT_SOURCE_DIR}/test/test_knowledge_base.hpp
                              ${CMAKE_CURRENT_SOURCE_DIR}/test/test_rank.hpp
                              ${CMAKE_CURRENT_SOURCE_DIR}/test/test_state.hpp
                              ${CMAKE_CURRENT_SOURCE_DIR}/test/test_asrtm.hpp
    )

# check if we need to add the test for the reward
if ( ENABLE_REWARD OR ENABLE_LEARNING )
        set ( ASRTM_TEST_HEADERS_TEMP ${ASRTM_TEST_HEADERS_TEMP} ${CMAKE_CURRENT_SOURCE_DIR}/test/test_reward.hpp )
endif ( ENABLE_REWARD OR ENABLE_LEARNING )
if ( ENABLE_LEARNING )
        set ( ASRTM_TEST_HEADERS_TEMP ${ASRTM_TEST_HEADERS_TEMP} ${CMAKE_CURRENT_SOURCE_DIR}/test/test_sw_mab.hpp )
        set ( ASRTM_TEST_HEADERS_TEMP ${ASRTM_TEST_HEADERS_TEMP} ${CMAKE_CURRENT_SOURCE_DIR}/test/test_mab_integration.hpp )
endif ( ENABLE_LEARNING )

# eventually set the complete suite of tests
set ( ASRTM_TEST_HEADERS ${ASRTM_TEST_HEADERS_TEMP} PARENT_SCOPE)
