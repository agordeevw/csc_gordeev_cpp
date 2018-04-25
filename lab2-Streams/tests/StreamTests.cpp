#include <sstream>
#include "gtest/gtest.h"
#include "Stream.h"

using namespace stream;

TEST(Stream, EmptyStream)
{
  Stream s_empty(std::list<int>{});
  std::ostringstream oss;

  s_empty | print_to(oss);
  EXPECT_EQ(oss.str(), std::string(""));
}
