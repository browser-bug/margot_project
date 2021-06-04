# Host flags
set(flags_release "-O3 -DNDEBUG -march=native -mtune=native")
set(flags_relwithdebinfo "-O2 -DNDEBUG -march=native -mtune=native -fno-omit-frame-pointer -fno-optimize-sibling-calls")
set(flags_debug "-O0 -g -Wall -pedantic")

# Release
set(CMAKE_C_FLAGS_RELEASE "${flags_release}" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELEASE "${flags_release}" CACHE STRING "")

# RelWithDebInfo
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${flags_relwithdebinfo}" CACHE STRING "")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${flags_relwithdebinfo}" CACHE STRING "")

# Debug
set(CMAKE_C_FLAGS_DEBUG "${flags_debug}" CACHE STRING "")
set(CMAKE_CXX_FLAGS_DEBUG "${flags_debug}" CACHE STRING "")
