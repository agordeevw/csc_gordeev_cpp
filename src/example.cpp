#include "EvictingCacheMap.h"

int main() {
  EvictingCacheMap<char, int> map(8);
  map.put('a', 2);
  map.put('b', 3);
  map.put('c', 4);
  map.put('d', 4);
  bool a = map.exists('a');  // must be false
  bool b = map.empty();      // must be false
  map.erase('c');
  bool c = map.exists('b');  // must be true
  auto existingValue = map.get('b');
  bool d = existingValue.has_value();  // must be true
  auto nonexistingValue = map.get('c');
  bool e = nonexistingValue.has_value();  // must be false
}