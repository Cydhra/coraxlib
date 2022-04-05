cmake_minimum_required (VERSION 3.0.2 FATAL_ERROR)

project (corax)

#set(PLL_EXPORTED_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src PARENT_SCOPE)

option(CORAX_BUILD_DOCS "Build documentation" OFF)
option(CORAX_BUILD_DIFFICULTY_PREDICTION "Build the difficulty prediction module" OFF)

# check if Doxygen is installed
if(CORAX_BUILD_DOCS)
  # Code taken from https://vicrucann.github.io/tutorials/quick-cmake-doxygen/
  find_package(Doxygen)
  if (DOXYGEN_FOUND)
      # set input and output files
      set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
      set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

      # request to configure the file
      configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
      message("Doxygen build started")

      # note the option ALL which allows to build the docs together with the application
      add_custom_target(doc_doxygen ALL
          COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          COMMENT "Generating API documentation with Doxygen"
          VERBATIM )
  else (DOXYGEN_FOUND)
    message("Doxygen need to be installed to generate the doxygen documentation")
  endif (DOXYGEN_FOUND)
endif(CORAX_BUILD_DOCS)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

set(CORAX_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/corax)
set(CORAX_SRC_DIR_CORE ${CORAX_SRC_DIR}/core)
set(CORAX_SRC_DIR_IO ${CORAX_SRC_DIR}/io)
set(CORAX_SRC_DIR_MODEL ${CORAX_SRC_DIR}/model)
set(CORAX_SRC_DIR_OPTIMIZE ${CORAX_SRC_DIR}/optimize)
set(CORAX_SRC_DIR_TREE ${CORAX_SRC_DIR}/tree)
set(CORAX_SRC_DIR_UTIL ${CORAX_SRC_DIR}/util)
set(CORAX_SRC_DIR_KERNEL ${CORAX_SRC_DIR}/kernel)
set(CORAX_SRC_DIR_KERNEL_GENERIC ${CORAX_SRC_DIR_KERNEL}/generic)
set(CORAX_SRC_DIR_KERNEL_SSE ${CORAX_SRC_DIR_KERNEL}/sse)
set(CORAX_SRC_DIR_KERNEL_AVX ${CORAX_SRC_DIR_KERNEL}/avx)
set(CORAX_SRC_DIR_KERNEL_AVX2 ${CORAX_SRC_DIR_KERNEL}/avx2)

# Export CORAX_LIBRARIES and CORAX_INCLUDE_DIRS as CACHE entries
set(CORAX_INCLUDE_DIRS
        ${PROJECT_SOURCE_DIR}/src
        CACHE INTERNAL "${PROJECT_NAME}: Include dirs")

if(CORAX_BUILD_DIFFICULTY_PREDICTION)
    add_subdirectory(lib/difficulty_prediction)
endif()

add_subdirectory(src/corax)

option(CORAX_TESTS_ENABLED "Enable and build the tests" ON)

if (CORAX_TESTS_ENABLED)
  enable_testing()
  add_subdirectory(test)
endif(CORAX_TESTS_ENABLED)


