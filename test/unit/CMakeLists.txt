find_package(GTest)


set(CORAX_TEST_SOURCES
  src/dummy.cpp
  src/newick.cpp
  src/main.cpp)

add_executable(corax-test ${CORAX_TEST_SOURCES})
target_include_directories(corax-test PRIVATE ${GTEST_INCLUDE_DIRS})
target_link_libraries(corax-test PRIVATE ${GTEST_BOTH_LIBRARIES} pthread corax_static)

if(${CMAKE_VERSION} VERSION_LESS "3.13") 
  set_target_properties(corax-test PROPERTIES COMPILE_FLAGS -fsanitize=address)
  set_target_properties(corax-test PROPERTIES LINK_FLAGS -fsanitize=address)
else()
  target_compile_options(corax-test PRIVATE -fsanitize=address)
  target_link_options(corax-test PRIVATE -fsanitize=address)
endif()

add_test(corax-test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/corax-test)
enable_testing()
