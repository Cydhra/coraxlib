#ifndef CORAXLIB_TEST_ENV_HPP
#define CORAXLIB_TEST_ENV_HPP

#include <fstream>
#include <iostream>

#include "gtest/gtest.h"

#define STRING(s) #s
#define STRINGIFY(s) STRING(s)

class CoraxlibEnvironment : public ::testing::Environment {
 public:
  CoraxlibEnvironment() {}

  std::string datafile_filename(){
    return std::string(STRINGIFY(TREEPATH/random_test_trees));
  }

  std::ifstream get_datafile() {
    return std::ifstream(STRINGIFY(TREEPATH/random_test_trees));
  }

  std::ifstream get_pathological_data() {
    return std::ifstream(STRINGIFY(TREEPATH/pathological_trees));
  }
};

extern CoraxlibEnvironment *env;


#endif
