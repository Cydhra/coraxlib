cmake_minimum_required(VERSION 3.0.2)

include(CheckIncludeFile) 

set (CORAX_BASE_FLAGS "-Wall -Wsign-compare -std=c99 -D_GNU_SOURCE -O3 -fPIC ")
set (CORAX_BASE_FLAGS_CXX "-Wall -Wsign-compare -std=c++11 -D_GNU_SOURCE -O3 -fPIC ")

if(CORAX_DEBUG)
  set(CORAX_BASE_FLAGS "${CORAX_BASE_FLAGS} -DDEBUG")
endif()

set (SSE_FLAGS "-msse3")
set (AVX_FLAGS "-mavx")
set (AVX2_FLAGS "-mfma -mavx2")

# needed for coraxmodule files
include_directories(${CORAX_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})


file(GLOB CORAX_OPTIMIZE_SOURCES ${CORAX_SRC_DIR_OPTIMIZE}/*.c
  ${CORAX_SRC_DIR_OPTIMIZE}/lbfgsb/*.c
)

file(GLOB CORAX_TREE_SOURCES ${CORAX_SRC_DIR_TREE}/*.c)

set(CORAX_SOURCES ${CORAX_SRC_DIR_CORE}/common.c
  ${CORAX_SRC_DIR_CORE}/parsimony.c
  ${CORAX_SRC_DIR_CORE}/partition.c
  ${CORAX_SRC_DIR_CORE}/repeats.c

  ${CORAX_SRC_DIR_IO}/binary.c
  ${CORAX_SRC_DIR_IO}/binary_io_operations.c
  ${CORAX_SRC_DIR_IO}/fasta.c
  ${CORAX_SRC_DIR_IO}/maps.c
  ${CORAX_SRC_DIR_IO}/output.c
  ${CORAX_SRC_DIR_IO}/phylip.c
  ${CORAX_SRC_DIR_IO}/utree_ascii.c
  ${CORAX_SRC_DIR_IO}/utree_newick.c
  ${CORAX_SRC_DIR_IO}/utree_svg.c
  ${CORAX_SRC_DIR_IO}/newick.cpp

  ${CORAX_SRC_DIR_KERNEL_GENERIC}/core_derivatives.c
  ${CORAX_SRC_DIR_KERNEL_GENERIC}/core_likelihood.c
  ${CORAX_SRC_DIR_KERNEL_GENERIC}/core_clv.c
  ${CORAX_SRC_DIR_KERNEL_GENERIC}/core_pmatrix.c
  ${CORAX_SRC_DIR_KERNEL_GENERIC}/fast_parsimony.c
  ${CORAX_SRC_DIR_KERNEL_GENERIC}/weighted_parsimony.c
  ${CORAX_SRC_DIR_KERNEL}/clv.c
  ${CORAX_SRC_DIR_KERNEL}/derivatives.c
  ${CORAX_SRC_DIR_KERNEL}/likelihood.c
  ${CORAX_SRC_DIR_KERNEL}/pmatrix.c

  ${CORAX_SRC_DIR_MODEL}/aa_rates.c
  ${CORAX_SRC_DIR_MODEL}/eigen.c
  ${CORAX_SRC_DIR_MODEL}/gamma.c
  ${CORAX_SRC_DIR_MODEL}/invariant.c
  ${CORAX_SRC_DIR_MODEL}/modutil.c
  ${CORAX_SRC_DIR_MODEL}/modutil_aa.c
  ${CORAX_SRC_DIR_MODEL}/modutil_dna.c
  ${CORAX_SRC_DIR_MODEL}/modutil_gt.c
  ${CORAX_SRC_DIR_MODEL}/modutil_mult.c
  ${CORAX_SRC_DIR_MODEL}/evolmodel.cpp
  
  ${CORAX_OPTIMIZE_SOURCES}
  
  ${CORAX_TREE_SOURCES}
  
  ${CORAX_SRC_DIR_UTIL}/compress.c
  ${CORAX_SRC_DIR_UTIL}/hardware.c
  ${CORAX_SRC_DIR_UTIL}/list.c
  ${CORAX_SRC_DIR_UTIL}/msa.c
  ${CORAX_SRC_DIR_UTIL}/random.c
  ${CORAX_SRC_DIR_UTIL}/stepwise.c
  )

file(GLOB CORAX_SSE_SOURCES ${CORAX_SRC_DIR_KERNEL_SSE}/core_derivatives_sse.c
  ${CORAX_SRC_DIR_KERNEL_SSE}/core_likelihood_sse.c
  ${CORAX_SRC_DIR_KERNEL_SSE}/core_clv_sse.c
  ${CORAX_SRC_DIR_KERNEL_SSE}/core_pmatrix_sse.c
  ${CORAX_SRC_DIR_KERNEL_SSE}/fast_parsimony_sse.c
  )

file(GLOB CORAX_AVX_SOURCES ${CORAX_SRC_DIR_KERNEL_AVX}/core_derivatives_avx.c
  ${CORAX_SRC_DIR_KERNEL_AVX}/core_likelihood_avx.c
  ${CORAX_SRC_DIR_KERNEL_AVX}/core_clv_avx.c
  ${CORAX_SRC_DIR_KERNEL_AVX}/core_pmatrix_avx.c
  ${CORAX_SRC_DIR_KERNEL_AVX}/fast_parsimony_avx.c
  )

file(GLOB CORAX_AVX2_SOURCES ${CORAX_SRC_DIR_KERNEL_AVX2}/core_derivatives_avx2.c
  ${CORAX_SRC_DIR_KERNEL_AVX2}/core_likelihood_avx2.c
  ${CORAX_SRC_DIR_KERNEL_AVX2}/core_clv_avx2.c
  ${CORAX_SRC_DIR_KERNEL_AVX2}/core_pmatrix_avx2.c
  ${CORAX_SRC_DIR_KERNEL_AVX2}/fast_parsimony_avx2.c
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

if (ENABLE_SSE OR ENABLE_AVX OR ENABLE_AVX2)
  add_definitions(-DHAVE_X86INTRIN_H)
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CORAX_BASE_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CORAX_BASE_FLAGS_CXX}")

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

option(CORAX_NONREV "Enable non-reversible model computation" OFF)

add_library(corax_obj OBJECT ${CORAX_SOURCES})
set_property(TARGET corax_obj PROPERTY POSITION_INDEPENDENT_CODE 1)
target_include_directories(corax_obj PUBLIC ${CORAX_INCLUDE_DIRS})

if(CORAX_NONREV)
  message("Non-rev enabled")
  target_compile_definitions(corax_obj PUBLIC -DCORAX_NONREV)
  target_link_libraries(corax_obj PUBLIC cblas lapack)
endif()

if(CORAX_BUILD_DIFFICULTY_PREDICTION)
  target_link_libraries(corax_obj PRIVATE coraxlib_difficulty_prediction_lib)
endif()

macro(add_corax_lib _target linkage)
    add_library(${_target} ${linkage} $<TARGET_OBJECTS:corax_obj>)
    set_target_properties(${_target} PROPERTIES OUTPUT_NAME "corax")
    set(CORAX_LIBRARIES 
      ${_target} ${CORAX_LIBRARIES}
      CACHE INTERNAL "${PROJECT_NAME}: Libraries to link against")
endmacro()

set(CORAX_LIBRARIES "")

if(BUILD_CORAX_SHARED)
  add_corax_lib(corax_shared SHARED)
endif()
if(BUILD_CORAX_STATIC)
  add_corax_lib(corax_static STATIC)
endif()

message(STATUS "coraxlib libraries: ${CORAX_LIBRARIES}")


#include(GNUInstallDirs)
#install(TARGETS corax_static ARCHIVE)
