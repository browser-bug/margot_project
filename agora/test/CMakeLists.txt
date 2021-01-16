set( AGORA_TEST_BINARY_NAME "test")

# define the related headers and source files
set( AGORA_TEST_BINARY_HEADERS
	)
set( AGORA_TEST_BINARY_SOURCES
	"test_launcher.cpp"
)

# declare the target
add_executable( ${AGORA_TEST_BINARY_NAME} ${AGORA_TEST_BINARY_SOURCES} ${AGORA_TEST_BINARY_HEADERS} )
add_executable( margot::${AGORA_TEST_BINARY_NAME} ALIAS ${AGORA_TEST_BINARY_NAME} )

# set the executable dependencies
target_link_libraries( ${AGORA_TEST_BINARY_NAME} PUBLIC margot::${AGORA_LIBRARY_NAME} Boost::program_options)

# set the runtime library path for executing it anywhere
set_target_properties(${AGORA_TEST_BINARY_NAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)


# define its compiler options
target_compile_features( ${AGORA_TEST_BINARY_NAME} PRIVATE cxx_std_17 ) # for the filesystem component
target_compile_options( ${AGORA_TEST_BINARY_NAME} PRIVATE "-Wall" )
target_compile_options( ${AGORA_TEST_BINARY_NAME} PRIVATE $<$<CONFIG:DEBUG>: -g -O0 >)
target_compile_options( ${AGORA_TEST_BINARY_NAME} PRIVATE $<$<CONFIG:RELWITHDEBINFO>: -march=native -mtune=native -g -O2 >)
target_compile_options( ${AGORA_TEST_BINARY_NAME} PRIVATE $<$<CONFIG:RELEASE>: -march=native -mtune=native -O3 >)
include(CheckIPOSupported)
check_ipo_supported(RESULT result)
if(result)
  set_target_properties( ${AGORA_TEST_BINARY_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE )
endif()
