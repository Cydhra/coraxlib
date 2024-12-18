include(targetSourcesLocal)
include(simd)

get_sources_local(
    CORAX_KERNEL_SOURCES_SSE
    core_clv_sse.c
    core_clv_sse.h
    core_derivatives_sse.c
    core_derivatives_sse.h
    core_likelihood_sse.c
    core_likelihood_sse.h
    core_pmatrix_sse.c
    core_pmatrix_sse.h
    fast_parsimony_sse.c
    fast_parsimony_sse.h
)

corax_add_simd_definitions("SSE" ${CORAX_LIB_TARGET} ${CORAX_KERNEL_SOURCES_SSE})

target_sources(${CORAX_LIB_TARGET} PRIVATE ${CORAX_KERNEL_SOURCES_SSE})
