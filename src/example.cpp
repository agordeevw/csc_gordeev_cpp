#include <iostream>

#include "EvictingCacheMap.h"

void print_bool(bool value, bool expected)
{
  std::cout << std::boolalpha
    << value << ", should be " <<  expected 
    << std::endl;
}

int main() {
  EvictingCacheMap<char, int> map(8);
  map.put('a', 2);
  map.put('b', 3);
  map.put('c', 4);
  map.put('d', 4);

  print_bool(map.exists('a'), true);
  print_bool(map.empty(), false);
  map.erase('c');
  print_bool(map.exists('b'), true);
  auto existingValue = map.get('b');
  print_bool(existingValue.has_value(), true);
  auto nonexistingValue = map.get('c');
  print_bool(nonexistingValue.has_value(), false);
}