# Lab2. C++ Stream library

## About

This header-only library provides lazy evaluation of different transformations on data sequences.

### Initialization

Stream can be initialized from:

* **Iterator range**: `Stream(Iterator begin, Iterator end)`. Values in range are not modified.
* **Container**: `Stream(const Container&), Stream(Container&&)`. Values in lvalue containers are not modified. If rvalue container is given, stream stores the container within itself.
* **Initializer list**: `Stream{x, y, z, ...}`
* **Pack**: `Stream(x, y, z, ...)`. All values must be of the same type.
* **Generator**: `Stream(Generator&&)`. Generator must a Callable object.

### Transformations

All stream operations are divided in two groups: operators and terminators. Operators transform the stream into another stream. Terminators transform the stream into anything else. Lazy evaluation in this case means that until terminator is applied to the stream, no calculations on data can happen. 

Assume S - stream, O - operators, T - terminator. Following transformation chains are valid:
* S | O \*( | O) : creates new stream transformed by operations, closing the initial stream.
* S | \* ( | O) | T : transforms the stream if operators are provided and terminates it into some value.
* O \* ( | O) : creates new composite operator that can be applied to streams.
* O \* ( | O) | T : creates new composite terminator that can be applied to streams.

Implemented terminators:

* **reduce(IdentityFn&&, Accumulator&&):** applies IdentityFn to first element of the stream and performs left fold on the rest of the stream using Accumulator as function and result os IdentityFn as initial element.
* **sum():** sums all values of the stream using value type's operator+.
* **print_to(std::ostream&, const char\*):** prints to std::ostream using string delimiter to split values.
* **to_vector():** forms std::vector from values of stream.
* **nth(size_t n):** returns nth element of stream.

Implemented operators:

* **get(size_t n):** returns stream formed from first n values of given stream.
* **skip(size_t n):** returns stream formed from given stream by skipping first n values.
* **map(Function&&):** applies Function to all stream values.
* **filter(Predicate&&):** forms new stream from all values of given stream that satisfy Predicate.
* **group(size_t size):** forms groups of stream values of fixed size. Last group might be undersized. 

### Examples 
Generate stream of all primes and print first 20: 
```
using namespace stream;

class Generator
{
public:
  Generator(int start) : value(start) {}
  int operator()() { return value++; }
private:
  int value;
};

auto isPrime = [](int x) {
  for (int y = 2; y <= x / 2; ++y)
    if (x % y == 0)
      return false;
  return true;
};

Stream stream(Generator(2));
auto primesStream = stream | filter(isPrime);
primesStream | get(20) | print_to(std::cout, ", ");
```

Fizzbuzz:
```
using namespace stream;

auto fizzbuzz = [] (int x) -> std::string {
  if (x % 3 == 0 && x % 5 == 0)
    return "Fizzbuzz";
  if (x % 3 == 0)
    return "Fizz";
  if (x % 5 == 0)
    return "Buzz";
  return std::to_string(x);
};

Stream fizzbuzzStream(Generator(1));
fizzbuzzStream | map(fizzbuzz) | get(30) | print_to(std::cout);
}
```

### Implementation notes

Streams can be empty, but their termination throws EmptyStreamExcaption.

Streams can be copied. In case of streams generated from rvalue containers the container is copied too.

Streams that were terminated or moved from are considered closed and cannot be used in calculations.
Any attempt to do so will lead to StreamClosedException throw.

Terminators and operators are implemented as named Callable types, allowing compile-time checks to validate stream transformation correctness. 

Some terminators technically do not support infinite streams. For example, application of to_vector() to Generator-based stream will lead to endless allocations. Applications of such terminators to infinite streams will lead to compilation errors to preven these kinds of mistakes.

For example:
```
Stream stream(InfiniteGenerator);
auto value = stream | nth(0);  // <- valid, nth doesn't require streams to be finite
// auto value = stream | sum();  // <- won't compile, cannot sum infinite streams
```

You can refer to ProjectStructure.cpp for overview of implementation structure and description of implementation details.

## Building

To build, run following command from the project root directory:

```
mkdir build && cd build && cmake .. -DBUILD_TESTS=ON && cmake --build . && cd ..
```

## Executing

Compiled binaries will be stored in bin folder. These binaries include two executables:

* Example - some examples of usage.
* StreamTests - self-explanatory.

## Checking

runchecks - small shell script which runs clang-analyzer, valgrind and gcov on source files and compiled binaries.
