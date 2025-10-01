include(targetSourcesLocal)

target_sources_local(
    ${CORAX_LIB_TARGET}
    PRIVATE
        au.h
        au.c
        erfinv.h
        erfinv.c
        bootstrap.h
        bootstrap.c
        stats.h
)
