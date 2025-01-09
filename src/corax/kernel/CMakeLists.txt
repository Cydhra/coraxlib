include(targetSourcesLocal)

target_sources_local(
    ${CORAX_LIB_TARGET}
    PRIVATE clv.c
            clv.h
            derivatives.c
            derivatives.h
            kernel_avx.h
            kernel_avx2.h
            kernel_generic.h
            kernel_sse.h
            likelihood.c
            likelihood.h
            pmatrix.c
            pmatrix.h
)

add_subdirectory(generic)

if (CORAX_ENABLE_SSE OR CORAX_ENABLE_SSE2NEON)
    add_subdirectory(sse)
endif ()

if (CORAX_ENABLE_AVX)
    add_subdirectory(avx)
endif ()

if (CORAX_ENABLE_AVX2)
    add_subdirectory(avx2)
endif ()
