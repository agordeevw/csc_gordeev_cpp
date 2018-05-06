#include <chrono>
#include <iostream>
#include <vector>
#include <type_traits>
#include <iostream>
#include <numeric>
#include <random>
#include <string>

#include "Stream.h"

namespace {

struct Incr {
  Incr(int start = 0) : counter(start) {}
  int counter = 0;
  int operator()() { return counter++; }
};

struct Rand {
  Rand(int min, int max) : min(min), max(max) {}
  int min, max;
  int operator()() { return rand() % (max - min) + min; }
};

int Fibo(int n) {
  if (n < 0) return 0;
  if (n == 0 || n == 1) return 1;
  return Fibo(n - 1) + Fibo(n - 2);
}

template <class T>
class CountedNoisy
{
public:
  CountedNoisy() {
    if (enabled)
      std::cout << "ctor " << id << '\n';
  }
  CountedNoisy(const CountedNoisy& other) {
    if (enabled)
      std::cout << "copy ctor " << id << " <- " << other.id << '\n';
  }
  CountedNoisy(CountedNoisy&& other) {
    if (enabled)
      std::cout << "move ctor " << id << " <- " << other.id << '\n';
  }
  CountedNoisy& operator=(const CountedNoisy& other) {
    if (enabled)
      std::cout << "copy asgn " << id << " <- " << other.id << '\n';
    return *this;
  }
  CountedNoisy& operator=(CountedNoisy&& other) {
    if (enabled)
      std::cout << "move asgn " << id << " <- " << other.id << '\n';
    return *this;
  }
  ~CountedNoisy() {
    if (enabled)
      std::cout << "dtor " << id << '\n';
  }
  size_t GetId() const {
    return id;
  }
  static void Mute() {
    CountedNoisy<T>::enabled = false;
  }
  static void Unmute() {
    CountedNoisy<T>::enabled = true;
  }

private:
  static thread_local bool enabled;
  static thread_local size_t lastId;
  size_t id = lastId++;
};

template <class T>
thread_local bool CountedNoisy<T>::enabled = true;

template <class T>
thread_local size_t CountedNoisy<T>::lastId = 0;

template <class T>
std::ostream& operator<<(std::ostream& os, const CountedNoisy<T>& value) {
  os << "id: " << value.GetId();
  return os;
}

template <class T>
class Noised : public CountedNoisy<Noised<T>>
{
public:
  Noised() {}
  Noised(const T& other) : value(other) {}
  Noised(T&& other) : value(std::move(other)) {}
  Noised& operator=(const T& other) {
    value = other;
    return *this;
  }
  Noised& operator=(T&& other) {
    value = std::move(other);
    return *this;
  }
  operator T() const { return value; }
  const T& Value() const { return value; }
  T& Value() { return value; }

private:
  T value;
};

template <class T>
std::ostream& operator<<(std::ostream& os, const Noised<T>& value) {
  os << value.Value() << " (";
  os << static_cast<const CountedNoisy<Noised<T>>&>(value) << ")";
  return os;
}

class GeneratorNoisedIncr {
public:
  GeneratorNoisedIncr(int start = 0) : 
    counter(start) {}
  Noised<int> operator()() { 
    return Noised<int>(counter++);
  }

private:
  int counter = 0;
};

struct GeneratorNoisedRand {
public:
  GeneratorNoisedRand(): 
    min(0), max(100000) {}
  GeneratorNoisedRand(int min, int max) : 
    min(min), max(max) {}
  Noised<int> operator()() {
    return Noised<int>(rand() % (max - min) + min);
  }

private:
  int min, max;
};

} // namespace 

int main(int, char**)
{
  using namespace stream;
  srand(std::time(0));

  try {
    {
      std::cout << "iterator init:\n";
      std::vector<int> v1{1, 2, 3};
      Stream s_range(v1.begin(), v1.end());
      (s_range 
          | print_to(std::cout)) << std::endl;

      std::cout << "lvalue container init:\n";
      Stream s_lvref(v1);
      (s_lvref 
          | print_to(std::cout)) << std::endl;

      std::cout << "const lvalue container init:\n";
      const auto& cv1 = v1;
      Stream s_clvref(cv1);
      (s_clvref 
          | print_to(std::cout)) << std::endl;

      std::cout << "rvalue container init:\n";
      Stream s_rref(std::move(v1));
      (s_rref
          | print_to(std::cout)) << std::endl;

      std::cout << "size of initial container:\n";
      std::cout << v1.size() << std::endl;
    }

    {
      std::cout << "Vector from generator:\n";
      auto vec = 
        Stream(Rand{0, 100})
          | get(2)
          | to_vector();

      Stream(std::move(vec))
      | print_to(std::cout);
      std::cout << std::endl;
    }

    {
      std::cout << "map&filter&print:\n";
      Stream s_map(std::vector<int>{1, 2, 3});
      s_map
        | map([](auto&& x) -> auto&& { return x; })
        | filter([](auto&& x) { return true; })
        | print_to(std::cout);
      std::cout << std::endl;
    }  
    
    {
      std::cout << "nth from generator:\n";
      auto val = 
        Stream(Rand{0, 100})
        | nth(4);
      std::cout << val << std::endl;
    }

    {
      Noised<int>::Mute();

      auto vec = 
        Stream(GeneratorNoisedRand{})
        | get(5)
        | to_vector();

      std::cout << "Values:\n";
      Stream(vec)
      | print_to(std::cout);
      std::cout << std::endl;

      std::cout << "Find min (Reduce):\n";
      auto val = 
        Stream(std::move(vec))
          | reduce(
              [] (auto&& min, auto&& val) -> auto&& { // remove "-> auto&&""
                                                      // unoptimized reduce
                return (val < min) ? val : min; 
              }
            );
      std::cout << val << std::endl;
    }    

    {
      std::cout << "initialization list:\n";
      Stream s_ilist({1, 2, 3});
      (s_ilist
          | print_to(std::cout)) << std::endl;
    }

    {
      std::cout << "generator:\n";
      Stream s_gen(Incr{});
      (s_gen 
          | get(7) 
          | print_to(std::cout)) << std::endl;
    }

    {
      std::cout << "pack:\n";
      Stream s_pack(1);
      (s_pack 
          | print_to(std::cout)) << std::endl;
    }

    {
      std::cout << "operator get:\n";
      Stream s_get(Rand{0, 100});
      (s_get 
          | get(5) 
          | print_to(std::cout)) << std::endl;
    }

    {
      std::cout << "operator map:\n";
      Stream s_map(1, 2, 3, 4, 5);
      (s_map 
          | map([](int x) { return x*x; })
          | print_to(std::cout)) << std::endl;
    }

    {
      std::cout << "operator reduce (expect 0):\n";
      Stream s_reduce(1, 1);
      std::cout << (s_reduce | reduce([](int x, int y) { return x - y; })) << std::endl;
    }

    {
      std::cout << "operator reduce with id (expect 1):\n";
      Stream s_reduce_id(1, 1);
      std::cout << 
        (s_reduce_id 
          | reduce([](int x) { return 2*x; }, 
                   [](int x, int y) { return x - y; })
        ) << std::endl;
    }

    {
      std::cout << "operator filter:\n";
      Stream s_filter(1, -1, 2, -2, 3, -3);
      (s_filter 
          | filter([](auto x){ return x > 0; }) 
          | print_to(std::cout)) << std::endl;
    }

    {
      std::cout << "operator skip:\n";
      Stream s_skip(0, 1, 2, 3, 4, 5);
      (s_skip 
          | skip(3) 
          | print_to(std::cout)) << std::endl;
    }

    {
      std::cout << "operator group:\n";
      Stream s_group(0,0,1,1,2,2,3);
      (s_group 
          | group(2) 
          | map([](auto&& vec) { return std::accumulate(vec.begin(), vec.end(), 0); }) 
          | print_to(std::cout)) << std::endl;
    }

    {
      std::cout << "operator sum:\n";
      Stream s_sum(1, 2, 3, 4, 5);
      std::cout << (s_sum | sum()) << std::endl;
    }

    {
      std::cout << "operator print_to:\n";
      Stream s_print(1, 2, 3, 4, 5);
      (s_print | print_to(std::cout, ", ")) << std::endl;
    }

    {
      std::cout << "operator to_vector:\n";
      Stream s_tovec(1, 2, 3, 4, 5);
      auto s_vec = s_tovec | to_vector();
      std::ostream_iterator<int> os_it(std::cout, " ");
      for (auto val : s_vec)
          os_it = val;
      std::cout << std::endl;
    }

    {
      std::cout << "operator nth (expect 1):\n";
      Stream s_nth(0,0,0,1,0);
      std::cout << (s_nth | nth(3)) << std::endl;
    }

    {
      std::cout << "positive even numbers:\n";
      Stream s_poseven(Incr{});
      (s_poseven
          | map([](int x) { return x % 2 == 0 ? x : -x; })
          | filter([](int x) { return x > 0; })
          | get(20)
          | print_to(std::cout)
          ) << std::endl;
    }

    {
      std::cout << "20 primes starting from 50th:\n";
      auto primesStream = 
          Stream(Incr{2})
          | filter([](int x) {
              for (int y = 2; y <= x / 2; ++y)
                  if (x % y == 0)
                      return false;
              return true;
            }
          );

      (primesStream | skip(50) | get(20) | print_to(std::cout)) << std::endl;

    }

    {
      std::cout << "10th fibo number:\n";
      auto fiboStream = 
          Stream(Incr{})
          | map([] (int x) { return Fibo(x); }
          );

      std::cout << (fiboStream | nth(10)) << std::endl;
    }

    {

      auto eulerRowStream = 
        Stream(Incr{})
        | map([] (int x) {
            if (x == 0) return 1.0;

            double ret = 1.0;
            for (int i = 1; i <= x; ++i)
              ret *= 1.0 / i;
            return ret;
          }
        );

      std::cout << "20th partial sum of euler row:\n";
      std::cout << (eulerRowStream | get(20) | sum()) << std::endl;
    }

    {
      std::cout << "fizzbuzz:\n";
      auto fizzbuzzStream = 
        Stream(Incr{1})
        | map([] (int x) -> std::string {
            if (x % 3 == 0 && x % 5 == 0)
              return "Fizzbuzz";
            if (x % 3 == 0)
              return "Fizz";
            if (x % 5 == 0)
              return "Buzz";
            return std::to_string(x);
          }
        );

      (fizzbuzzStream | get(30) | print_to(std::cout)) << std::endl;

    }

    {
      auto randomData =
        Stream(Rand{0, 10})
        | get(10)
        | to_vector();

      std::cout << "Random values:\n";

      (Stream(randomData)
        | print_to(std::cout)) << std::endl;

      std::cout << "Minimal of these values:\n";

      std::cout << (
        Stream(randomData)
          | reduce([](int min, int x) {
              return (x < min) ? x : min;
            })
        ) << std::endl;

      std::cout << "Sum of these values:\n";

      std::cout << (
        Stream(randomData)
          | sum()
        ) << std::endl;

      std::cout << "Average of these values:\n";

      class AverageReduce {
      public:
        AverageReduce() : counter(1) {};
        double operator()(double result, double value) {
          double temp = result * counter;
          ++counter;
          temp += value;
          return temp / counter;
        }

      private:
        int counter;
      };

      std::cout << (
        Stream(randomData)
          | reduce(
              [](int x) -> double { return x; }, 
              AverageReduce()
            )
        ) << std::endl;

    }

    {

      std::cout << "Composite operators:\n";

      auto transformAndFilter = 
        map([](int x) { return x % 2 == 0 ? x : -x; })
        | filter([](int x) { return x > 0; });

      auto groupAndSum = 
        group(3)
        | map(
            [](auto&& vec) {
              return std::accumulate(vec.begin(), vec.end(), 0);
            }
          )
        | sum();

      auto value = 
        Stream({1, 2, 3, 4, 5, 6, 7, 8, 9})
          | transformAndFilter
          | groupAndSum;

      std::cout << value << std::endl;
    }
  }
  catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}
