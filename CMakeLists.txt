project(libdvbcsa)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -Wall -Wpedantic")

# Build with cmake -DCMAKE_DEBUG_TYPE=Debug
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -DDVBCSA_DEBUG")

# Build with cmake -DCMAKE_DEBUG_TYPE=Release
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3 -fomit-frame-pointer")

option(uint32 "Use native 32 bits integers for bitslice" OFF)
option(uint64 "Use native 64 bits integers for bitslice" OFF)
option(mmx "Use MMX for bitslice" OFF)
option(sse2 "Use SSE2 for bitslice" OFF)
option(altivec "Use AltiVec for bitslice" OFF)

include(CheckTypeSize)
check_type_size(long SIZEOF_LONG)

if(mmx)
  set(transpose_64 YES)
  set(DVBCSA_USE_MMX 1)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmmx")
elseif(sse2)
  set(transpose_128 YES)
  set(DVBCSA_USE_SSE 1)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -msse -msse2")
elseif(altivec)
  set(transpose_128 YES)
  set(DVBCSA_USE_ALTIVEC 1)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -maltivec -mabi=altivec")
elseif(uint32)
  set(transpose_32 YES)
  set(DVBCSA_USE_UINT32 1)
elseif(uint64)
  set(transpose_64 YES)
  set(DVBCSA_USE_UINT64 1)
elseif(SIZEOF_LONG MATCHES 8)
  set(transpose_64 YES)
  set(DVBCSA_USE_UINT64 1)
else()
  set(transpose_32 YES)
  set(DVBCSA_USE_UINT32 1)
endif()

include(CheckIncludeFiles)

check_include_files("sys/stat.h" HAVE_SYS_STAT_H)
check_include_files("sys/types.h" HAVE_SYS_TYPES_H)
check_include_files(assert.h HAVE_ASSERT_H)
check_include_files(dlfcn.h HAVE_DLFCN_H)
check_include_files(inttypes.h HAVE_INTTYPES_H)
check_include_files(memory.h HAVE_MEMORY_H)
check_include_files(stdint.h HAVE_STDINT_H)
check_include_files(stdlib.h HAVE_STDLIB_H)
check_include_files(string.h HAVE_STRING_H)
check_include_files(strings.h HAVE_STRINGS_H)
check_include_files(unistd.h HAVE_UNISTD_H)
check_include_files("stdlib.h;stdarg.h;string.h;float.h" STDC_HEADERS)

include(CheckCXXSourceRuns)

check_cxx_source_runs("
        #include <mm_malloc.h>
        int main()
        {
          void* mem = _mm_malloc (100, 16);
          return 0;
        }"
  HAVE_MM_MALLOC)

check_cxx_source_runs("
        #include <stdlib.h>
        int main()
        {
          void* mem;
          return posix_memalign (&mem, 16, 100);
        }"
  HAVE_POSIX_MEMALIGN)

configure_file(${PROJECT_SOURCE_DIR}/config.h.cmake ${PROJECT_BINARY_DIR}/config.h)
include_directories(${PROJECT_BINARY_DIR})

macro (add_sources)
    file (RELATIVE_PATH _relPath "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
    foreach (_src ${ARGN})
        if (_relPath)
            list (APPEND SRC_FILES "${_relPath}/${_src}")
        else()
            list (APPEND SRC_FILES "${_src}")
        endif()
    endforeach()
    if (_relPath)
        # propagate SRCS to parent directory
        set (SRC_FILES ${SRC_FILES} PARENT_SCOPE)
    endif()
endmacro()

add_subdirectory(src)

add_library(
  dvbcsa
  SHARED
  ${SRC_FILES}
  )

set(DVBCSA_VERSION_MAJOR 1)
set(DVBCSA_VERSION_MINOR 1)
set(DVBCSA_VERSION_PATCH 0)
set(DVBCSA_VERSION_STRING ${DVBCSA_VERSION_MAJOR}.${DVBCSA_VERSION_MINOR}.${DVBCSA_VERSION_PATCH})

set_target_properties(dvbcsa PROPERTIES
  VERSION ${DVBCSA_VERSION_STRING}
  SOVERSION ${DVBCSA_VERSION_MAJOR})

install(TARGETS dvbcsa DESTINATION lib)
install(FILES src/dvbcsa/dvbcsa.h DESTINATION include/dvbcsa)

add_subdirectory(test)
