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

if(CORAX_ENABLE_SSE) 
    corax_add_simd_definitions("SSE" ${CORAX_LIB_TARGET} ${CORAX_KERNEL_SOURCES_SSE})
elseif(CORAX_ENABLE_SSE2NEON)
    # atm either SSE or SSE2NEON kernels can be built, since the share the same function and object names
    corax_add_simd_definitions("SSE2NEON" ${CORAX_LIB_TARGET} ${CORAX_KERNEL_SOURCES_SSE})
endif()

target_sources(${CORAX_LIB_TARGET} PRIVATE ${CORAX_KERNEL_SOURCES_SSE})
