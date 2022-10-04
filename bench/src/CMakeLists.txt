# Load GoogleBenchmark
find_package(benchmark REQUIRED)

# Register the corax-bench executable.
add_executable(corax-bench)
target_sources_local(
    corax-bench PRIVATE PLLTreeInfo.cpp PLLUnrootedTree.cpp LibpllParsers.cpp Model.cpp bench.cpp
)

target_compile_definitions(corax-bench PRIVATE BENCH_DATA=${CORAX_BENCH_DATA_DIR})
target_include_directories(corax-bench PRIVATE ${GBENCH_INCLUDE_DIRS} ${CORAX_INCLUDE_DIRS})
target_link_libraries(corax-bench PRIVATE ${CORAX_LIB_TARGET} benchmark::benchmark)
