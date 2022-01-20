find_package(benchmark REQUIRED)

set(CORAX_BENCH_SOURCES
  bench.cpp)


set(CORAX_BENCH_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)
add_executable(corax-bench ${CORAX_BENCH_SOURCES})
target_compile_definitions(corax-bench PRIVATE BENCH_DATA=${CORAX_BENCH_DATA_DIR})
target_include_directories(corax-bench PRIVATE ${GBENCH_INCLUDE_DIRS} ${CORAX_INCLUDE_DIRS})
#target_link_libraries(corax-test PRIVATE ${GBENCH_BOTH_LIBRARIES} pthread)
target_link_libraries(corax-bench PRIVATE ${CORAX_LIBRARIES})
target_link_libraries(corax-bench PRIVATE benchmark::benchmark)

