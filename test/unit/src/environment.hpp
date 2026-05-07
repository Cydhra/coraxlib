#ifndef CORAXLIB_TEST_ENV_HPP
#define CORAXLIB_TEST_ENV_HPP

#include <fstream>

#include "gtest/gtest.h"

#define STRING(s) #s
#define STRINGIFY(s) STRING(s)

class CoraxlibEnvironment : public ::testing::Environment {
 public:
  CoraxlibEnvironment() {}

  std::string datapath(const std::string filename) {
      return std::string(STRINGIFY(DATAPATH)) + "/" + filename;
  }
};

extern CoraxlibEnvironment *env;


#endif
