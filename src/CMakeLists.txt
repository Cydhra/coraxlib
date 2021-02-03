cmake_minimum_required(VERSION 3.0.2)

include(CheckIncludeFile) 

set (CORAX_BASE_FLAGS "-Wall -Wsign-compare -D_GNU_SOURCE -O3 -fPIC ")

if(CORAX_DEBUG)
  set(CORAX_BASE_FLAGS "${CORAX_BASE_FLAGS} -DDEBUG")
endif()

set (SSE_FLAGS "-msse3")
set (AVX_FLAGS "-mavx")
set (AVX2_FLAGS "-mfma -mavx2")

find_package(BISON)
find_package(FLEX)
set(CORAX_BISON_FLAGS "-y -d -p pll_utree_")
set(CORAX_FLEX_FLAGS "-P pll_utree_")
BISON_TARGET(parse_utree_t 
  ${CMAKE_CURRENT_SOURCE_DIR}/parse_utree.y ${CMAKE_CURRENT_BINARY_DIR}/parse_utree.c
  COMPILE_FLAGS ${CORAX_BISON_FLAGS})
FLEX_TARGET(lex_utree_t 
  ${CMAKE_CURRENT_SOURCE_DIR}/lex_utree.l ${CMAKE_CURRENT_BINARY_DIR}/lex_utree.c
  COMPILE_FLAGS ${CORAX_FLEX_FLAGS})
ADD_FLEX_BISON_DEPENDENCY(lex_utree_t parse_utree_t)
set(CORAX_BISON_FLAGS "-y -d -p pll_rtree_")
set(CORAX_FLEX_FLAGS "-P pll_rtree_")
BISON_TARGET(parse_rtree_t 
  ${CMAKE_CURRENT_SOURCE_DIR}/parse_rtree.y ${CMAKE_CURRENT_BINARY_DIR}/parse_rtree.c
  COMPILE_FLAGS ${CORAX_BISON_FLAGS})
FLEX_TARGET(lex_rtree_t 
  ${CMAKE_CURRENT_SOURCE_DIR}/lex_rtree.l ${CMAKE_CURRENT_BINARY_DIR}/lex_rtree.c
  COMPILE_FLAGS ${CORAX_FLEX_FLAGS})
ADD_FLEX_BISON_DEPENDENCY(lex_rtree_t parse_rtree_t)

# needed for bison files
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#remove annoyting bison and flex warnings
SET_SOURCE_FILES_PROPERTIES( ${FLEX_lex_utree_t_OUTPUTS} PROPERTIES COMPILE_FLAGS -Wno-sign-compare )
SET_SOURCE_FILES_PROPERTIES( ${FLEX_lex_rtree_t_OUTPUTS} PROPERTIES COMPILE_FLAGS -Wno-sign-compare )

set(CORAX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/compress.c
  ${BISON_parse_utree_t_OUTPUTS}
  ${FLEX_lex_utree_t_OUTPUTS}
  ${BISON_parse_rtree_t_OUTPUTS}
  ${FLEX_lex_rtree_t_OUTPUTS}
  ${CMAKE_CURRENT_SOURCE_DIR}/core_derivatives.c
  ${CMAKE_CURRENT_SOURCE_DIR}/core_likelihood.c
  ${CMAKE_CURRENT_SOURCE_DIR}/core_clv.c
  ${CMAKE_CURRENT_SOURCE_DIR}/core_pmatrix.c
  ${CMAKE_CURRENT_SOURCE_DIR}/derivatives.c
  ${CMAKE_CURRENT_SOURCE_DIR}/fasta.c
  ${CMAKE_CURRENT_SOURCE_DIR}/fast_parsimony.c
  ${CMAKE_CURRENT_SOURCE_DIR}/gamma.c
  ${CMAKE_CURRENT_SOURCE_DIR}/hardware.c
  ${CMAKE_CURRENT_SOURCE_DIR}/likelihood.c
  ${CMAKE_CURRENT_SOURCE_DIR}/list.c
  ${CMAKE_CURRENT_SOURCE_DIR}/maps.c
  ${CMAKE_CURRENT_SOURCE_DIR}/models.c
  ${CMAKE_CURRENT_SOURCE_DIR}/output.c
  ${CMAKE_CURRENT_SOURCE_DIR}/parsimony.c
  ${CMAKE_CURRENT_SOURCE_DIR}/clv.c
  ${CMAKE_CURRENT_SOURCE_DIR}/phylip.c
  ${CMAKE_CURRENT_SOURCE_DIR}/pll.c
  ${CMAKE_CURRENT_SOURCE_DIR}/random.c
  ${CMAKE_CURRENT_SOURCE_DIR}/repeats.c
  ${CMAKE_CURRENT_SOURCE_DIR}/rtree.c
  ${CMAKE_CURRENT_SOURCE_DIR}/stepwise.c
  ${CMAKE_CURRENT_SOURCE_DIR}/utree.c
  ${CMAKE_CURRENT_SOURCE_DIR}/utree_moves.c
  ${CMAKE_CURRENT_SOURCE_DIR}/utree_svg.c
  ${CMAKE_CURRENT_SOURCE_DIR}/newick/newick.cpp
  )

file(GLOB CORAX_SSE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/core_derivatives_sse.c
  ${CMAKE_CURRENT_SOURCE_DIR}/core_likelihood_sse.c
  ${CMAKE_CURRENT_SOURCE_DIR}/core_clv_sse.c
  ${CMAKE_CURRENT_SOURCE_DIR}/core_pmatrix_sse.c
  ${CMAKE_CURRENT_SOURCE_DIR}/fast_parsimony_sse.c
  )

file(GLOB CORAX_AVX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/core_derivatives_avx.c
  ${CMAKE_CURRENT_SOURCE_DIR}/core_likelihood_avx.c
  ${CMAKE_CURRENT_SOURCE_DIR}/core_clv_avx.c
  ${CMAKE_CURRENT_SOURCE_DIR}/core_pmatrix_avx.c
  ${CMAKE_CURRENT_SOURCE_DIR}/fast_parsimony_avx.c
  )

file(GLOB CORAX_AVX2_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/core_derivatives_avx2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/core_likelihood_avx2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/core_clv_avx2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/core_pmatrix_avx2.c
  ${CMAKE_CURRENT_SOURCE_DIR}/fast_parsimony_avx2.c
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

add_library(corax_obj OBJECT ${CORAX_SOURCES})

set(CORAX_LIBRARIES "")
  
# Build shared library
if(BUILD_CORAX_SHARED)
  message(STATUS "Coraxlib shared build enabled")
  set_property(TARGET corax_obj PROPERTY POSITION_INDEPENDENT_CODE 1) 
  add_library(corax_shared  SHARED $<TARGET_OBJECTS:pll_obj>)
  target_include_directories(corax_shared INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
  set(CORAX_LIBRARIES
    corax_shared
    CACHE INTERNAL "${PROJECT_NAME}: Libraries to link against")
endif()

# Build static library
if(BUILD_CORAX_STATIC)
  message(STATUS "Coraxlib static build enabled")
  add_library(corax_static STATIC $<TARGET_OBJECTS:corax_obj>)
  target_include_directories(corax_static INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
  set(CORAX_LIBRARIES 
    corax_static ${CORAX_LIBRARIES}
    CACHE INTERNAL "${PROJECT_NAME}: Libraries to link against")
endif()

message(STATUS "${CORAX_LIBRARIES}")
set_target_properties(${CORAX_LIBRARIES} PROPERTIES OUTPUT_NAME "corax")


# Export CORAX_LIBRARIES and CORAX_INCLUDE_DIRS as CACHE entries
set(CORAX_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}
  CACHE INTERNAL "${PROJECT_NAME}: Include dirs")


