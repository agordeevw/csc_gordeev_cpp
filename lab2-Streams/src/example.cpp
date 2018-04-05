#include <iostream>
#include <vector>

#include "Stream.h"

int main(int, char**)
{
  using namespace stream;

  try {
    std::vector<int> vec{1,2,3,4,5};
    Stream s(vec.begin(), vec.end());
    auto res = s | nth(5);
    std::cout << res << std::endl;
  }
  catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}
