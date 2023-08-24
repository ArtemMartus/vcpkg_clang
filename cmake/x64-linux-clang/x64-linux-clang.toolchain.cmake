include_guard(GLOBAL)

option(VCPKG_USE_LTO "Enable full LTO for release builds" OFF)

set(CMAKE_USER_MAKE_RULES_OVERRIDE "${CMAKE_CURRENT_LIST_DIR}/Platform/clang-override.cmake")
set(CMAKE_USER_MAKE_RULES_OVERRIDE_C "${CMAKE_CURRENT_LIST_DIR}/Platform/clang-c.cmake")
set(CMAKE_USER_MAKE_RULES_OVERRIDE_CXX "${CMAKE_CURRENT_LIST_DIR}/Platform/clang-cxx.cmake")

# Set C standard.
set(CMAKE_C_STANDARD 11 CACHE STRING "")
set(CMAKE_C_STANDARD_REQUIRED ON CACHE STRING "")
set(CMAKE_C_EXTENSIONS ON CACHE STRING "")

# Set C++ standard.
set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
set(CMAKE_CXX_STANDARD_REQUIRED ON CACHE STRING "")
set(CMAKE_CXX_EXTENSIONS OFF CACHE STRING "")

# general architecture flags
set(arch_flags "-march=native")

set(CMAKE_C_COMPILER "/usr/bin/clang-10" CACHE STRING "")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++-10" CACHE STRING "")
set(CMAKE_LINKER "/usr/bin/lld-10" CACHE STRING "")

# Set compiler flags.

set(CLANG_C_LTO_FLAGS "-fuse-ld=lld-10")
set(CLANG_CXX_LTO_FLAGS "-fuse-ld=lld-10")
if(VCPKG_USE_LTO)
  set(CLANG_C_LTO_FLAGS "-flto -fuse-ld=lld-10")
  set(CLANG_CXX_LTO_FLAGS "-flto -fuse-ld=lld-10 -fwhole-program-vtables")
endif()

set(CMAKE_C_FLAGS "${arch_flags} ${VCPKG_C_FLAGS} ${CLANG_C_LTO_FLAGS}" CACHE STRING "")

set(CMAKE_CXX_FLAGS "${arch_flags} ${VCPKG_CXX_FLAGS} ${CLANG_CXX_LTO_FLAGS}" CACHE STRING "")

# Setup try_compile correctly. Requires all variables required by the toolchain. 
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES VCPKG_C_FLAGS VCPKG_CXX_FLAGS
                                                 VCPKG_USE_LTO
                                                 VCPKG_TARGET_TRIPLET
                                                 )

# Remove variables which are not needed anymore
unset(CLANG_C_LTO_FLAGS)
unset(CLANG_CXX_LTO_FLAGS)
unset(arch_flags)
