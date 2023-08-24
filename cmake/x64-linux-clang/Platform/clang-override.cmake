# This module is shared by multiple languages; use include blocker.
if(__CLANG_OVERRIDE)
  return()
endif()
set(__CLANG_OVERRIDE 1)

if(NOT DEFINED CMAKE_LINKER)
  set(CMAKE_LINKER lld-10)
endif()

# default to Debug builds
set(CMAKE_BUILD_TYPE_INIT Debug)

set(CMAKE_CXX_STANDARD_LIBRARIES_INIT "${CMAKE_C_STANDARD_LIBRARIES_INIT}")

macro(compiler_clang lang)
  set(CMAKE_LINK_PCH ON)
  message("clang-override CMAKE_${lang}_COMPILER_ID="${CMAKE_${lang}_COMPILER_ID})
  if (CMAKE_${lang}_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_PCH_PROLOGUE "#pragma clang system_header")
  endif()
endmacro()
