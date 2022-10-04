include(CheckIncludeFile)
include(targetSourcesLocal)

# Add the corax library
set(CORAX_LIB_TARGET
    "corax"
    CACHE INTERNAL "${PROJECT_NAME}: Library target to link against."
)
# STATIC / SHARED is auto-choosen based on BUILD_SHARED_LIBS
add_library(${CORAX_LIB_TARGET})
target_sources(${CORAX_LIB_TARGET} PRIVATE ${CORAX_SOURCES})

# For other programs to include coraxlib, it has to have position independent code.
if (BUILD_SHARED_LIBS)
    set_property(TARGET ${CORAX_LIB_TARGET} PROPERTY POSITION_INDEPENDENT_CODE 1)
endif ()

# Add sources and recurse into subdirs
target_sources_local(${CORAX_LIB_TARGET}
    PUBLIC
        corax_core.h
        corax_io.h
        corax_kernel.h
        corax_model.h
        corax_optimize.h
        corax_tree.h
        corax_util.h
        corax.hpp
)
add_subdirectory(core)
add_subdirectory(optimize)
add_subdirectory(tree)
add_subdirectory(io)
add_subdirectory(kernel)
add_subdirectory(model)
add_subdirectory(util)

# Add include directories.
target_include_directories(${CORAX_LIB_TARGET} PRIVATE ${CORAX_INCLUDE_DIRS})

# Add warning flags
target_compile_options(${CORAX_LIB_TARGET} PRIVATE ${CORAX_WARNING_FLAGS})

# TODO Decide if we want to use DEBUG, NDEBUG, or DEBUG_MODE and unify all usages.
if (CORAX_DEBUG)
    target_compile_definitions(${CORAX_LIB_TARGET} PRIVATE "DEBUG")
endif ()

# We want GNU instead of POSIX behaviour.
target_compile_definitions(${CORAX_LIB_TARGET} PRIVATE "_GNU_SOURCE")

# If the difficulty prediction is enabled, link it to the corax_obj.
if (CORAX_BUILD_DIFFICULTY_PREDICTION)
    target_link_libraries(${CORAX_LIB_TARGET} PRIVATE coraxlib_difficulty_prediction_lib)
endif ()

# Set the SIMD flags corresponding to the requested and supported variant.
if (CORAX_ENABLE_SSE)
    target_add_sse_definitions(${CORAX_LIB_TARGET})
endif ()

if (CORAX_ENABLE_AVX)
    target_add_avx_definitions(${CORAX_LIB_TARGET})
endif ()

if (CORAX_ENABLE_AVX2)
    target_add_avx2_definitions(${CORAX_LIB_TARGET})
endif ()

# Set the library name to "corax", regardless of if we build the static or shared version. We
# currently do not need this, as we call both of them "corax".
# set_target_properties(${CORAX_LIB_TARGET} PROPERTIES OUTPUT_NAME "corax")
