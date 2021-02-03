find_package(GTest)


set(LIBPLL_TEST_SOURCES
  src/dummy.cpp
  src/newick.cpp
  src/main.cpp)

add_executable(libpll-test ${LIBPLL_TEST_SOURCES})
target_include_directories(libpll-test PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(libpll-test PRIVATE ${GTEST_BOTH_LIBRARIES} pthread pll_static)

if(${CMAKE_VERSION} VERSION_LESS "3.13") 
  set_target_properties(libpll-test PROPERTIES COMPILE_FLAGS -fsanitize=address)
  set_target_properties(libpll-test PROPERTIES LINK_FLAGS -fsanitize=address)
else()
  target_compile_options(libpll-test PRIVATE -fsanitize=address)
  target_link_options(libpll-test PRIVATE -fsanitize=address)
endif()

add_test(libpll-test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/libpll-test)
enable_testing()
