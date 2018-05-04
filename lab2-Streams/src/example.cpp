#include <iostream>
#include <vector>
#include <type_traits>
#include <iostream>

#include "Stream.h"

struct Incr {
  int counter = 0;
  int operator()() { return counter++; }
};

struct Rand {
  int operator()() { return rand(); }
};

int main(int, char**)
{
  using namespace stream;

  try {
    std::cout << "iterator init:\n";
    std::vector<int> v{1, 2, 3, 4, 5};
    Stream s_range(v.begin(), v.end());
    (s_range | print_to(std::cout)) << std::endl;

    std::cout << "lvalue container init:\n";
    Stream s_lvref(v);
    (s_lvref | print_to(std::cout)) << std::endl;

    std::cout << "rvalue container init:\n";
    Stream s_rref(std::vector<int>{1, 2, 3, 4});
    (s_rref | print_to(std::cout)) << std::endl;

    std::cout << "initialization list:\n";
    Stream s_ilist({1, 2, 3});
    (s_ilist | print_to(std::cout)) << std::endl;

    std::cout << "generator:\n";
    Stream s_gen(Incr{});
    (s_gen | get(7) | print_to(std::cout)) << std::endl;

    /*std::cout << "pack:\n";
    Stream s_pack(1);
    (s_pack | print_to(std::cout)) << std::endl;

    std::cout << "operator get:\n";
    Stream s_get(Rand{});
    (s_get | get(5) | print_to(std::cout)) << std::endl;

    std::cout << "operator map:\n";
    Stream s_map(1, 2, 3, 4, 5);
    (s_map | map([](int x) { return x*x; }) | print_to(std::cout)) << std::endl;

    std::cout << "operator reduce:\n";
    Stream s_reduce(1, 1);
    std::cout << (s_reduce | reduce([](int x, int y) { return x - y; })) << std::endl;

    std::cout << "operator reduce with id:\n";
    Stream s_reduce_id(1, 1);
    std::cout << 
      (s_reduce_id | reduce(
        [](int x) { return 2*x; }, 
        [](int x, int y) { return x - y; })
      ) << std::endl;

    std::cout << "operator filter:\n";
    Stream s_filter(1, -1, 2, -2, 3, -3);
    (s_filter | filter([](auto x){ return x > 0; }) | print_to(std::cout)) << std::endl;

    std::cout << "operator skip:\n";
    Stream s_skip(0, 0, 0, 1, 1, 1);
    (s_skip | skip(3) | print_to(std::cout)) << std::endl;

    std::cout << "operator group:\n";
    Stream s_group(0,0,1,1,2,2,3);
    (s_group | group(2) | print_to(std::cout)) << std::endl;

    std::cout << "operator sum:\n";
    Stream s_sum(1, 2, 3, 4, 5);
    std::cout << (s_sum | sum()) << std::endl;

    std::cout << "operator print_to:\n";
    Stream s_print(1, 2, 3, 4, 5);
    (s_print | print_to(std::cout, ", ")) << std::endl;

    std::cout << "operator to_vector:\n";
    Stream s_tovec(1, 2, 3, 4, 5);
    auto s_vec = s_tovec | to_vector();
    std::ostream_iterator<int> os_it(std::cout, ", ");
    for (auto val : s_vec)
        os_it = val;
    std::cout << std::endl;

    std::cout << "operator nth:\n";
    Stream s_nth(0,0,0,1,0);
    std::cout << (s_nth | nth(3)) << std::endl;*/
  }
  catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}
