# Flags to enable the different SIMD variants.
set(SSE_FLAGS "-msse3")
set(AVX_FLAGS "-mavx")
set(AVX2_FLAGS "-mfma -mavx2")
set(NATIVE_FLAGS "-march=native")

if (NOT DEFINED CORAX_BUILD_PORTABLE_ARCH)
  string(REPLACE "x86_64" "x86-64" CORAX_BUILD_PORTABLE_ARCH ${CMAKE_SYSTEM_PROCESSOR})    
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(NOSIMD_FLAGS "-march=${CORAX_BUILD_PORTABLE_ARCH}")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(NOSIMD_FLAGS "-fno-slp-vectorize -fno-vectorize -march=${CORAX_BUILD_PORTABLE_ARCH}")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    set(NOSIMD_FLAGS "-no-vec -march=pentium")
else()
    # Unknown compiler, fall back to -O1
    set(NOSIMD_FLAGS "-O1")
endif()

# Check if the compiler, OS, and architecture support the different SIMD variants. It seems like the
# only reasonable way to check if a SIMD variant is supported by the whole toolchain is to actually
# compile small test programs.
function (check_sse_available)
    set(TEST_CODE " #include <immintrin.h>
        int main() {__m128d a = _mm_setzero_pd();  return 1;}"
    )
    set(TEST_FILE ${CMAKE_CURRENT_BINARY_DIR}/test_sse.c)
    file(WRITE "${TEST_FILE}" "${TEST_CODE}")
    try_compile(
        SSE_COMPILED ${CMAKE_CURRENT_BINARY_DIR}
        ${TEST_FILE}
        COMPILE_DEFINITIONS ${SSE_FLAGS}
    )
    set(SSE_AVAILABLE
        ${SSE_COMPILED}
        PARENT_SCOPE
    )
endfunction ()

function (check_avx_available)
    set(TEST_CODE " #include <immintrin.h>
        int main() {__m256d a = _mm256_setzero_pd ();  return 1;}"
    )
    set(TEST_FILE ${CMAKE_CURRENT_BINARY_DIR}/test_avx.c)
    file(WRITE "${TEST_FILE}" "${TEST_CODE}")
    try_compile(
        AVX_COMPILED ${CMAKE_CURRENT_BINARY_DIR}
        ${TEST_FILE}
        COMPILE_DEFINITIONS ${AVX_FLAGS}
    )
    set(AVX_AVAILABLE
        ${AVX_COMPILED}
        PARENT_SCOPE
    )
endfunction ()

function (check_avx2_available)
    set(TEST_CODE " #include <immintrin.h>
        int main() {__m256i a, b; b =  _mm256_abs_epi16(a); return 1;}"
    )
    set(TEST_FILE ${CMAKE_CURRENT_BINARY_DIR}/test_avx2.c)
    file(WRITE "${TEST_FILE}" "${TEST_CODE}")
    try_compile(
        AVX2_COMPILED ${CMAKE_CURRENT_BINARY_DIR}
        ${TEST_FILE}
        COMPILE_DEFINITIONS ${AVX2_FLAGS}
    )
    set(AVX2_AVAILABLE
        ${AVX2_COMPILED}
        PARENT_SCOPE
    )
endfunction ()

# Add compile definitions for the different SIMD variants.
function(__corax_get_simd_definitions SIMD_VARIANT)
    set(_SIMD_DEFS "HAVE_${SIMD_VARIANT}" "HAVE_X86INTRIN_H")
    if (SIMD_VARIANT STREQUAL "NONE")
      set(_SIMD_FLAGS ${NOSIMD_FLAGS})
      set(_SIMD_DEFS "")
    elseif (SIMD_VARIANT STREQUAL "NATIVE")
      set(_SIMD_FLAGS ${NATIVE_FLAGS})
      set(_SIMD_DEFS "HAVE_AUTOVEC")
    elseif (SIMD_VARIANT STREQUAL "SSE" OR SIMD_VARIANT STREQUAL "SSE3")
      set(_SIMD_FLAGS ${SSE_FLAGS})
    elseif (SIMD_VARIANT STREQUAL "AVX")
      set(_SIMD_FLAGS ${AVX_FLAGS})
    elseif (SIMD_VARIANT STREQUAL "AVX2")
      set(_SIMD_FLAGS ${AVX2_FLAGS})
    endif()
    set(SIMD_DEFS ${_SIMD_DEFS} PARENT_SCOPE)
    set(SIMD_FLAGS ${_SIMD_FLAGS} PARENT_SCOPE)
endfunction()

function(corax_add_simd_definitions SIMD_VARIANT TARGET)
    __corax_get_simd_definitions(${SIMD_VARIANT})
#    message(STATUS "SIMD_VARIANT: ${SIMD_VARIANT} ")
#    message(STATUS "SIMD_DEFS: ${SIMD_DEFS} ")
#    message(STATUS "SIMD_FLAGS: ${SIMD_FLAGS} ")
#    message(STATUS "SOURCES: ${ARGN}")
    set_source_files_properties(${ARGN} TARGET_DIRECTORY ${TARGET} 
       PROPERTIES COMPILE_FLAGS ${SIMD_FLAGS} 
                  COMPILE_DEFINITIONS "${SIMD_DEFS}"
    )
endfunction()


function(__corax_target_simd_definitions TARGET SIMD_VARIANT)
    __corax_get_simd_definitions(${SIMD_VARIANT})
    target_compile_definitions(${TARGET} PRIVATE ${SIMD_DEFS})
    target_compile_options(${TARGET} PRIVATE ${SIMD_FLAGS})
endfunction()

function (target_add_vnone_definitions TARGET)
    __corax_target_simd_definitions(${TARGET} "NONE")
endfunction ()

function (target_add_vnative_definitions TARGET)
    __corax_target_simd_definitions(${TARGET} "NATIVE")
endfunction ()

function (target_add_sse_definitions TARGET)
    __corax_target_simd_definitions(${TARGET} SSE)
endfunction ()

function (target_add_avx_definitions TARGET)
    __corax_target_simd_definitions(${TARGET} AVX)
endfunction ()

function (target_add_avx2_definitions TARGET)
    __corax_target_simd_definitions(${TARGET} AVX2)
endfunction ()
