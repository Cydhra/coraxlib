cmake_minimum_required(VERSION 3.0.2)

include(CheckIncludeFile) 

set (CORAX_BASE_FLAGS "-Wall -Wsign-compare -D_GNU_SOURCE -O3 -fPIC ")

if(CORAX_DEBUG)
  set(CORAX_BASE_FLAGS "${CORAX_BASE_FLAGS} -DDEBUG")
endif()

set (SSE_FLAGS "-msse3")
set (AVX_FLAGS "-mavx")
set (AVX2_FLAGS "-mfma -mavx2")

set(CORAX_SRC_DIR_CORE ${CMAKE_CURRENT_SOURCE_DIR}/core)
set(CORAX_SRC_DIR_CORE_TREE ${CORAX_SRC_DIR_CORE}/tree)
set(CORAX_SRC_DIR_CORE_IO ${CORAX_SRC_DIR_CORE}/io)
set(CORAX_SRC_DIR_CORE_MODEL ${CORAX_SRC_DIR_CORE}/model)
set(CORAX_SRC_DIR_CORE_UTIL ${CORAX_SRC_DIR_CORE}/util)
set(CORAX_SRC_DIR_CORE_KERNEL ${CORAX_SRC_DIR_CORE}/kernel)
set(CORAX_SRC_DIR_CORE_KERNEL_GENERIC ${CORAX_SRC_DIR_CORE_KERNEL}/generic)
set(CORAX_SRC_DIR_CORE_KERNEL_SSE ${CORAX_SRC_DIR_CORE_KERNEL}/sse)
set(CORAX_SRC_DIR_CORE_KERNEL_AVX ${CORAX_SRC_DIR_CORE_KERNEL}/avx)
set(CORAX_SRC_DIR_CORE_KERNEL_AVX2 ${CORAX_SRC_DIR_CORE_KERNEL}/avx2)


find_package(BISON)
find_package(FLEX)
set(CORAX_BISON_FLAGS "-y -d -p pll_utree_")
set(CORAX_FLEX_FLAGS "-P pll_utree_")

set(CORAX_BISON_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/core/tree)
file(MAKE_DIRECTORY ${CORAX_BISON_OUT_DIR})

BISON_TARGET(parse_utree_t 
  ${CORAX_SRC_DIR_CORE_TREE}/parse_utree.y ${CORAX_BISON_OUT_DIR}/parse_utree.c
  COMPILE_FLAGS ${CORAX_BISON_FLAGS})
FLEX_TARGET(lex_utree_t 
  ${CORAX_SRC_DIR_CORE_TREE}/lex_utree.l ${CORAX_BISON_OUT_DIR}/lex_utree.c
  COMPILE_FLAGS ${CORAX_FLEX_FLAGS})
ADD_FLEX_BISON_DEPENDENCY(lex_utree_t parse_utree_t)
set(CORAX_BISON_FLAGS "-y -d -p pll_rtree_")
set(CORAX_FLEX_FLAGS "-P pll_rtree_")
BISON_TARGET(parse_rtree_t 
  ${CORAX_SRC_DIR_CORE_TREE}/parse_rtree.y ${CORAX_BISON_OUT_DIR}/parse_rtree.c
  COMPILE_FLAGS ${CORAX_BISON_FLAGS})
FLEX_TARGET(lex_rtree_t 
  ${CORAX_SRC_DIR_CORE_TREE}/lex_rtree.l ${CORAX_BISON_OUT_DIR}/lex_rtree.c
  COMPILE_FLAGS ${CORAX_FLEX_FLAGS})
ADD_FLEX_BISON_DEPENDENCY(lex_rtree_t parse_rtree_t)

# needed for bison files
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#remove annoyting bison and flex warnings
SET_SOURCE_FILES_PROPERTIES( ${FLEX_lex_utree_t_OUTPUTS} PROPERTIES COMPILE_FLAGS -Wno-sign-compare )
SET_SOURCE_FILES_PROPERTIES( ${FLEX_lex_rtree_t_OUTPUTS} PROPERTIES COMPILE_FLAGS -Wno-sign-compare )

set(CORAX_SOURCES ${CORAX_SRC_DIR_CORE}/common.c
  ${CORAX_SRC_DIR_CORE}/partition.c
  ${CORAX_SRC_DIR_CORE}/repeats.c

  ${CORAX_SRC_DIR_CORE_IO}/fasta.c
  ${CORAX_SRC_DIR_CORE_IO}/maps.c
  ${CORAX_SRC_DIR_CORE_IO}/output.c
  ${CORAX_SRC_DIR_CORE_IO}/phylip.c
  ${CORAX_SRC_DIR_CORE_IO}/utree_svg.c
  ${CORAX_SRC_DIR_CORE_IO}/newick/newick.cpp

  ${CORAX_SRC_DIR_CORE_KERNEL_GENERIC}/core_derivatives.c
  ${CORAX_SRC_DIR_CORE_KERNEL_GENERIC}/core_likelihood.c
  ${CORAX_SRC_DIR_CORE_KERNEL_GENERIC}/core_clv.c
  ${CORAX_SRC_DIR_CORE_KERNEL_GENERIC}/core_pmatrix.c
  ${CORAX_SRC_DIR_CORE_KERNEL_GENERIC}/fast_parsimony.c
  ${CORAX_SRC_DIR_CORE_KERNEL}/clv.c
  ${CORAX_SRC_DIR_CORE_KERNEL}/derivatives.c
  ${CORAX_SRC_DIR_CORE_KERNEL}/likelihood.c
  ${CORAX_SRC_DIR_CORE_KERNEL}/parsimony.c

  ${CORAX_SRC_DIR_CORE_MODEL}/gamma.c
  ${CORAX_SRC_DIR_CORE_MODEL}/models.c

  ${BISON_parse_utree_t_OUTPUTS}
  ${FLEX_lex_utree_t_OUTPUTS}
  ${BISON_parse_rtree_t_OUTPUTS}
  ${FLEX_lex_rtree_t_OUTPUTS}
  ${CORAX_SRC_DIR_CORE_TREE}/rtree.c
  ${CORAX_SRC_DIR_CORE_TREE}/utree.c
  ${CORAX_SRC_DIR_CORE_TREE}/utree_moves.c
  
  ${CORAX_SRC_DIR_CORE_UTIL}/compress.c
  ${CORAX_SRC_DIR_CORE_UTIL}/hardware.c
  ${CORAX_SRC_DIR_CORE_UTIL}/list.c
  ${CORAX_SRC_DIR_CORE_UTIL}/random.c
  ${CORAX_SRC_DIR_CORE_UTIL}/stepwise.c
  )

file(GLOB CORAX_SSE_SOURCES ${CORAX_SRC_DIR_CORE_KERNEL_SSE}/core_derivatives_sse.c
  ${CORAX_SRC_DIR_CORE_KERNEL_SSE}/core_likelihood_sse.c
  ${CORAX_SRC_DIR_CORE_KERNEL_SSE}/core_clv_sse.c
  ${CORAX_SRC_DIR_CORE_KERNEL_SSE}/core_pmatrix_sse.c
  ${CORAX_SRC_DIR_CORE_KERNEL_SSE}/fast_parsimony_sse.c
  )

file(GLOB CORAX_AVX_SOURCES ${CORAX_SRC_DIR_CORE_KERNEL_AVX}/core_derivatives_avx.c
  ${CORAX_SRC_DIR_CORE_KERNEL_AVX}/core_likelihood_avx.c
  ${CORAX_SRC_DIR_CORE_KERNEL_AVX}/core_clv_avx.c
  ${CORAX_SRC_DIR_CORE_KERNEL_AVX}/core_pmatrix_avx.c
  ${CORAX_SRC_DIR_CORE_KERNEL_AVX}/fast_parsimony_avx.c
  )

file(GLOB CORAX_AVX2_SOURCES ${CORAX_SRC_DIR_CORE_KERNEL_AVX2}/core_derivatives_avx2.c
  ${CORAX_SRC_DIR_CORE_KERNEL_AVX2}/core_likelihood_avx2.c
  ${CORAX_SRC_DIR_CORE_KERNEL_AVX2}/core_clv_avx2.c
  ${CORAX_SRC_DIR_CORE_KERNEL_AVX2}/core_pmatrix_avx2.c
  ${CORAX_SRC_DIR_CORE_KERNEL_AVX2}/fast_parsimony_avx2.c
  )

# check that user did not disable simd
if (NOT DEFINED ENABLE_SSE)
  SET(ENABLE_SSE "True")
endif ()
if (NOT DEFINED ENABLE_AVX)
  SET(ENABLE_AVX "True")
endif ()
if (NOT DEFINED ENABLE_AVX2)
  SET(ENABLE_AVX2 "True")
endif ()

# check simd installed 
if (ENABLE_SSE)
  SET(_code " #include <immintrin.h>
  int main() {__m128d a = _mm_setzero_pd();  return 1;}")
  SET(_file ${CMAKE_CURRENT_BINARY_DIR}/testsse.c)
  FILE(WRITE "${_file}" "${_code}")
  TRY_COMPILE(SSE_COMPILED ${CMAKE_CURRENT_BINARY_DIR} ${_file}
    COMPILE_DEFINITIONS ${SSE_FLAGS})
  if (NOT SSE_COMPILED)
    message(STATUS "Disable SSE simd, because not supported") 
    set(ENABLE_SSE "False")
  endif()
endif()
if (ENABLE_AVX)
  SET(_code " #include <immintrin.h>
  int main() {__m256d a = _mm256_setzero_pd ();  return 1;}")
  SET(_file ${CMAKE_CURRENT_BINARY_DIR}/testavx.c)
  FILE(WRITE "${_file}" "${_code}")
  TRY_COMPILE(AVX_COMPILED ${CMAKE_CURRENT_BINARY_DIR} ${_file}
    COMPILE_DEFINITIONS ${AVX_FLAGS})
  if (NOT AVX_COMPILED)
    message(STATUS "Disable AVX simd, because not supported") 
    set(ENABLE_AVX "False")
  endif()
endif()
if (ENABLE_AVX2)
  SET(_code " #include <immintrin.h>
  int main() {__m256i a, b; b =  _mm256_abs_epi16(a); return 1;}")
  SET(_file ${CMAKE_CURRENT_BINARY_DIR}/testavx2.c)
  FILE(WRITE "${_file}" "${_code}")
  TRY_COMPILE(AVX2_COMPILED ${CMAKE_CURRENT_BINARY_DIR} ${_file}
    COMPILE_DEFINITIONS ${AVX2_FLAGS})
  if (NOT AVX2_COMPILED)
    message(STATUS "Disable AVX2 simd, because not supported") 
    set(ENABLE_AVX2 "False")
  endif()
endif()


# set simd flags
if (ENABLE_SSE)
  add_definitions(-DHAVE_SSE3)
  set(SIMD_FLAGS "${SIMD_FLAGS} ${SSE_FLAGS}")
  message(STATUS "SSE enabled. To disable it, run cmake with -DENABLE_SSE=false")
  set(CORAX_SOURCES ${CORAX_SOURCES} ${CORAX_SSE_SOURCES})
  SET_SOURCE_FILES_PROPERTIES( ${CORAX_SSE_SOURCES} PROPERTIES COMPILE_FLAGS ${SSE_FLAGS} )
endif ()
if (ENABLE_AVX)
  add_definitions(-DHAVE_AVX)
  set(SIMD_FLAGS "${SIMD_FLAGS} ${AVX_FLAGS}")
  message(STATUS "AVX enabled. To disable it, run cmake with -DENABLE_AVX=false")
  set(CORAX_SOURCES ${CORAX_SOURCES} ${CORAX_AVX_SOURCES})
  SET_SOURCE_FILES_PROPERTIES( ${CORAX_AVX_SOURCES} PROPERTIES COMPILE_FLAGS ${AVX_FLAGS} )
endif ()
if (ENABLE_AVX2)
  add_definitions(-DHAVE_AVX2)
  set(SIMD_FLAGS "${SIMD_FLAGS} ${AVX2_FLAGS}")
  message(STATUS "AVX2 enabled. To disable it, run cmake with -DENABLE_AVX2=false")
  set(CORAX_SOURCES ${CORAX_SOURCES} ${CORAX_AVX2_SOURCES})
  SET_SOURCE_FILES_PROPERTIES( ${CORAX_AVX2_SOURCES} PROPERTIES COMPILE_FLAGS ${AVX2_FLAGS} )
endif ()

add_definitions(-DHAVE_X86INTRIN_H)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CORAX_BASE_FLAGS}")

# Set BUILD_CORAX_SHARED and BUILD_CORAX_SHARED if both are unset
# Use BUILD_SHARED_LIBS to decide whether we should build shared or static libraries
if(NOT DEFINED BUILD_CORAX_SHARED AND NOT DEFINED BUILD_CORAX_STATIC)
  set(BUILD_CORAX_SHARED ${BUILD_SHARED_LIBS})
  if (BUILD_SHARED_LIBS)
    set(BUILD_CORAX_STATIC FALSE)
  else()
    set(BUILD_CORAX_STATIC TRUE)
  endif()
endif()

macro(add_corax_lib _target _sources)
  add_library(${_target}_obj OBJECT ${_sources})
  set_property(TARGET ${_target}_obj PROPERTY POSITION_INDEPENDENT_CODE 1)
  target_include_directories(${_target}_obj PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
  if(BUILD_CORAX_SHARED)
    add_library(${_target}_shared SHARED $<TARGET_OBJECTS:${_target}_obj>)
    set_target_properties(${_target}_shared PROPERTIES OUTPUT_NAME "${_target}")
    if(NOT ${_target} STREQUAL "corax")
      target_link_libraries(${_target}_shared corax_shared)
    endif() 
    set(CORAX_LIBRARIES 
      ${_target}_shared ${CORAX_LIBRARIES}
      CACHE INTERNAL "${PROJECT_NAME}: Libraries to link against")
  endif()
  if(BUILD_CORAX_STATIC)
    add_library(${_target}_static STATIC $<TARGET_OBJECTS:${_target}_obj>)
    set_target_properties(${_target}_static PROPERTIES OUTPUT_NAME "${_target}")
    if(NOT ${_target} STREQUAL "corax")
      target_link_libraries(${_target}_static corax_static) 
    endif()
    set(CORAX_LIBRARIES 
      ${_target}_static ${CORAX_LIBRARIES}
      CACHE INTERNAL "${PROJECT_NAME}: Libraries to link against")
  endif()
endmacro()

set(CORAX_LIBRARIES "")

add_corax_lib(corax "${CORAX_SOURCES}")

#set(CORAX_COMPONENTS "optimize;algorithm;binary;msa;tree;util" CACHE STRING "corax components to build")
set(CORAX_COMPONENTS "msa" CACHE STRING "corax components to build")

foreach(module ${CORAX_COMPONENTS})
  message(STATUS "Will compile corax-component ${module}")
  add_subdirectory(${module})
endforeach()
  
message(STATUS "coraxlib libraries: ${CORAX_LIBRARIES}")

# Export CORAX_LIBRARIES and CORAX_INCLUDE_DIRS as CACHE entries
set(CORAX_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  CACHE INTERNAL "${PROJECT_NAME}: Include dirs")


