# Lab2. C++ Stream library

The library provides lazy evaluation of different transformations on data sequences.

Stream can be initialized from:

* Iterator range
* Container
* Initializer list
* Pack (all values must be of the same type)

All stream operations are divided in two groups: operators and terminators. Operators transform the stream into another stream. Terminators transform the stream into anything else.

Lazy evaluation in this case means that until terminator is applied to the stream, no calculations on data can happen.

Implemented terminators:

* reduce
* sum
* print_to - prints to std::ostream
* to_vector
* nth

Implemented operators:

* get
* skip
* map
* filter
* group

You can refer to ProjectStructure.cpp for overview of implementation structure and description of implementation details.

## Building

To build:

```
mkdir build
cd build
cmake .. -DBUILD_TESTS=ON
cmake --build .
```

## Executing

Compiled binaries will be stored in bin folder.

* Example - some examples of usage.
* StreamTests - self-explanatory.

## Checking

runchecks - small shell script which runs clang-analyzer, valgrind and gcov on source files and compiled binaries.