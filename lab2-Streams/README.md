# Lab2. C++ Stream library

This header-only library provides lazy evaluation of different transformations on data sequences.

Stream can be initialized from:

* Iterator range
* Container
* Initializer list
* Pack (all values must be of the same type)
* Generator - Callable

All stream operations are divided in two groups: operators and terminators. Operators transform the stream into another stream. Terminators transform the stream into anything else.

Lazy evaluation in this case means that until terminator is applied to the stream, no calculations on data can happen. 

Implemented terminators:

* reduce(IdentityFn&&, Accumulator&&) - applies IdentityFn to first element of the stream and performs left fold on the rest of the stream using Accumulator.
* sum - sums all values of the stream using operator+
* print_to(std::ostream&, const char* delim) - prints to std::ostream using delimiter to split values
* to_vector - forms std::vector from values of stream
* nth(size_t n) - returns nth element of stream

Implemented operators:

* get(size_t n) - returns stream formed from first n values of given stream.
* skip(size_t n) - returns stream formed from given by skipping first n values.
* map(Function&&) - applies Function to all stream values
* filter(Predicate&&) - forms new stream from all values of given stream that satisfy Predicate
* group(size_t) - forms groups of stream values of fixed size. Last group might be undersized. 

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
