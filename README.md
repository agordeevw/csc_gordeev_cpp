# LRU-cache

Implementation of LRU evicting cache data structure based on hash map.

## Building

To build, run these commands:

```
mkdir build
cd build
cmake .. -DBUILD_TESTS=ON
cmake --build .
```

## Executing

Compiled binaries will be stored in bin folder.

* example - small showcase of usage
* EvictingCacheMapUnitTests - unit tests for the data structure

## Checking

runchecks - small shell script which runs cppcheck, clang-analyzer, valgrind and gcov on source files and compiled binaries.