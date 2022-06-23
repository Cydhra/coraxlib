include(CoraxlibTestHelper)
include(GoogleTest)

set(CORAX_TEST_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/data)

corax_register_test(test_dummy FILES src/dummy.cpp)
corax_register_test(test_newick FILES src/newick.cpp DEFINES TREEPATH=${CORAX_TEST_DATA_DIR})
corax_register_test(test_evolmodel FILES src/evolmodel.cpp)
corax_register_test(test_nni FILES src/nni.cpp)
corax_register_test(test_partition_create FILES src/partition_create.cpp DEFINES HAVE_SSE3=1 HAVE_AVX=1 HAVE_AVX2=0)

enable_testing()
