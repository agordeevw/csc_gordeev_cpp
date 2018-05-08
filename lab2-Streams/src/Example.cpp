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

class GeneratorIncr
{
public:
  GeneratorIncr(int start = 0) : counter(start) {}
  int operator()() { return counter++; }
private:
  int counter = 0;
};

class GeneratorRand
{
public:
  GeneratorRand(int min, int max) : min(min), max(max) {}
  int operator()() { return rand() % (max - min) + min; }
private:
  int min, max;
};

int Fibo(int n) {
  if (n < 0) return 0;
  if (n == 0 || n == 1) return 1;
  return Fibo(n - 1) + Fibo(n - 2);
}

using namespace stream;

void MakeRandomVector() {
  Stream stream(GeneratorRand{0, 100});
  auto vec = stream | get(10) | to_vector();

  std::cout << "Vector made from random generator:\n";
  Stream print(vec);
  print | print_to(std::cout);
  std::cout << std::endl;
}

void MakePrimes() {
  std::cout << "20 primes starting from 50th:\n";

  auto isPrime = [](int x) {
    for (int y = 2; y <= x / 2; ++y)
        if (x % y == 0)
            return false;
    return true;
  };

  Stream stream(GeneratorIncr{2});
  stream | filter(isPrime) | skip(50) | get(20) | print_to(std::cout);
  std::cout << std::endl;
}

void MakeFiboNumbers() {
  std::cout << "10th fibo number:\n";

  Stream stream(GeneratorIncr{});
  auto val = stream | map([] (int x) { return Fibo(x); }) | nth(10);

  std::cout << val << std::endl;
}

void MakeEulerPartialSum() {
  std::cout << "20th partial sum of euler row:\n";

  auto getEulerRowSummand = [] (int x) {
    if (x == 0) return 1.0;

    double ret = 1.0;
    for (int i = 1; i <= x; ++i)
      ret *= 1.0 / i;
    return ret;
  };

  Stream stream(GeneratorIncr{});
  auto value = stream | map(getEulerRowSummand) | get(20) | sum();

  std::cout << value << std::endl;
}

void MakeFizzbuzz() {
  std::cout << "Fizzbuzz:\n";

  auto fizzbuzz = [] (int x) -> std::string {
    if (x % 3 == 0 && x % 5 == 0)
      return "Fizzbuzz";
    if (x % 3 == 0)
      return "Fizz";
    if (x % 5 == 0)
      return "Buzz";
    return std::to_string(x);
  };

  Stream fizzbuzzStream(GeneratorIncr{1});
  fizzbuzzStream | map(fizzbuzz) | get(30) | print_to(std::cout);
  std::cout << std::endl;
}

void MakeMinSumAverage() {
  Stream randomGeneratorStream(GeneratorRand{0, 10});
  auto randomData =
    randomGeneratorStream | get(10) | to_vector();

  std::cout << "Random values:\n";

  Stream printStream(randomData);
  (printStream | print_to(std::cout)) << std::endl;

  std::cout << "Minimal of these values (reduce):\n";

  Stream minvalueStream(randomData);
  auto minValue = minvalueStream
    | reduce([](int x, int y) { return std::min(x, y); });
  std::cout << minValue << std::endl;

  std::cout << "Sum of these values:\n";

  Stream sumStream(randomData);
  std::cout << (sumStream | sum()) << std::endl;

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

  Stream averageStream(randomData);
  auto averageValue = averageStream
    | reduce([](int x) -> double { return x; }, AverageReduce{});
  std::cout << averageValue << std::endl;
}

}  // namespace

int main(int, char**)
{
  using namespace stream;
  srand(std::time(0));

  try {
    MakeRandomVector();
    MakePrimes();
    MakeFiboNumbers();
    MakeEulerPartialSum();
    MakeFizzbuzz();
    MakeMinSumAverage();
  }
  catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}
