include(targetSourcesLocal)

get_sources_local(
    CORAX_KERNEL_SOURCES_AVX2
    core_clv_avx2.c
    core_clv_avx2.h
    core_derivatives_avx2.c
    core_derivatives_avx2.h
    core_likelihood_avx2.c
    core_likelihood_avx2.h
    core_pmatrix_avx2.c
    core_pmatrix_avx2.h
    fast_parsimony_avx2.c
    fast_parsimony_avx2.h
)

corax_add_simd_definitions("AVX2" ${CORAX_LIB_TARGET} ${CORAX_KERNEL_SOURCES_AVX2})

target_sources(${CORAX_LIB_TARGET} PRIVATE ${CORAX_KERNEL_SOURCES_AVX2})
