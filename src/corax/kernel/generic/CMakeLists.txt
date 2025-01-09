include(targetSourcesLocal)

get_sources_local(
    CORAX_KERNEL_SOURCES_GENERIC
    core_clv.c
    core_clv.h
    core_derivatives.c
    core_derivatives.h
    core_likelihood.c
    core_likelihood.h
    core_pmatrix.c
    core_pmatrix.h
    fast_parsimony.c
    fast_parsimony.h
    weighted_parsimony.c
    weighted_parsimony.h
)

if (CORAX_ENABLE_AUTOVEC_KERNELS)
  set(GENERIC_SIMD "NATIVE")
else()
  set(GENERIC_SIMD "NONE")
endif()

corax_add_simd_flags(${GENERIC_SIMD} ${CORAX_LIB_TARGET} ${CORAX_KERNEL_SOURCES_GENERIC})

target_sources(${CORAX_LIB_TARGET} PRIVATE ${CORAX_KERNEL_SOURCES_GENERIC})
