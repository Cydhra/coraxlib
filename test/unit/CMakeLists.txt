find_package(GTest)


set(CORAX_TEST_SOURCES
  src/dummy.cpp
  src/newick.cpp
  src/main.cpp)

set(CORAX_TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)

add_executable(corax-test ${CORAX_TEST_SOURCES})
target_include_directories(corax-test PRIVATE ${GTEST_INCLUDE_DIRS} ${CORAX_INCLUDE_DIRS})
target_link_libraries(corax-test PRIVATE ${GTEST_BOTH_LIBRARIES} pthread)
target_link_libraries(corax-test PRIVATE ${CORAX_LIBRARIES})
target_compile_definitions(corax-test PRIVATE TREEPATH=${CORAX_TEST_DATA_DIR})

if(${CMAKE_VERSION} VERSION_LESS "3.13") 
  set_target_properties(corax-test PROPERTIES COMPILE_FLAGS -fsanitize=address)
  set_target_properties(corax-test PROPERTIES LINK_FLAGS -fsanitize=address)
else()
  target_compile_options(corax-test PRIVATE -fsanitize=address)
  target_link_options(corax-test PRIVATE -fsanitize=address)
endif()

add_test(corax-test ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/corax-test)
enable_testing()
