set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE static)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Linux)
set(VCPKG_BUILD_TYPE release)
set(VCPKG_TARGET_TRIPLET x64-linux-clang-10)

string(APPEND VCPKG_C_FLAGS "-march=native")
string(APPEND VCPKG_CXX_FLAGS "-march=native")

set(CMAKE_LINKER "/usr/bin/lld-link-10")
set(CMAKE_C_COMPILER "/usr/bin/clang-10")
set(CMAKE_CXX_COMPILER "/usr/bin/clang++-10")

message("${VCPKG_TARGET_TRIPLET} toolchain CMAKE_C_COMPILER = ${CMAKE_C_COMPILER}")
message("${VCPKG_TARGET_TRIPLET} toolchain CMAKE_CXX_COMPILER = ${CMAKE_CXX_COMPILER}")
message("${VCPKG_TARGET_TRIPLET} toolchain CMAKE_LINKER = ${CMAKE_LINKER}")
