set(flags_release "-O3 -DNDEBUG -mcpu=native -ggdb")
set(flags_relwithdebinfo "-O2 -DNDEBUG -mcpu=native -fno-omit-frame-pointer -fno-optimize-sibling-calls -ggdb")
set(flags_debug "-O0 -g")

# Release
set(CMAKE_C_FLAGS_RELEASE "${flags_release}" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELEASE "${flags_release}" CACHE STRING "")

# RelWithDebInfo
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${flags_relwithdebinfo}" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${flags_relwithdebinfo}" CACHE STRING "")

# Debug
set(CMAKE_C_FLAGS_DEBUG "${flags_debug}" CACHE STRING "")
set(CMAKE_CXX_FLAGS_DEBUG "${flags_debug}" CACHE STRING "")
