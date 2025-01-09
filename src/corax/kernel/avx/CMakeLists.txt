include(targetSourcesLocal)

get_sources_local(
    CORAX_KERNEL_SOURCES_AVX
    core_clv_avx.c
    core_clv_avx.h
    core_derivatives_avx.c
    core_derivatives_avx.h
    core_likelihood_avx.c
    core_likelihood_avx.h
    core_pmatrix_avx.c
    core_pmatrix_avx.h
    fast_parsimony_avx.c
    fast_parsimony_avx.h
)

corax_add_simd_flags("AVX" ${CORAX_LIB_TARGET} ${CORAX_KERNEL_SOURCES_AVX})

target_sources(${CORAX_LIB_TARGET} PRIVATE ${CORAX_KERNEL_SOURCES_AVX})
