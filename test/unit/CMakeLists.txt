find_package(GTest)


set(LIBPLL_TEST_SOURCES
  src/dummy.cpp
  src/newick.cpp
  src/main.cpp)

add_executable(libpll-test ${LIBPLL_TEST_SOURCES})
target_include_directories(libpll-test PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(libpll-test PRIVATE ${GTEST_BOTH_LIBRARIES} pll_static)
target_compile_options(libpll-test PRIVATE -fsanitize=address)
target_link_options(libpll-test PRIVATE -fsanitize=address)

add_test(libpll-test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libpll-test)
enable_testing()
