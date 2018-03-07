#ifndef INCLUDE_EVICTINGCACHEMAP_H_
#define INCLUDE_EVICTINGCACHEMAP_H_

#include <algorithm>
#include <cstddef>
#include <functional>
#include <list>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

template <class TKey, class TValue, class THash = std::hash<TKey>>
class EvictingCacheMap final {
  using PairsList = std::list<std::pair<const TKey, TValue>>;
  using IteratorsList = std::list<typename PairsList::iterator>;
  using HashTable = std::vector<IteratorsList>;

 public:
  using iterator = typename PairsList::iterator;
  using const_iterator = typename PairsList::const_iterator;
  /**
   * Construct a EvictingCacheMap
   * @param capacity maximum size of the cache map.  Once the map size exceeds
   *    maxSize, the map will begin to evict.
   */
  explicit EvictingCacheMap(std::size_t capacity) : capacity(capacity) {
    if (capacity == 0)
      throw std::logic_error("Unable to create cache of size 0");

    const std::size_t initialTableSize = 4;
    hashTable.resize(initialTableSize);
  }

  EvictingCacheMap(const EvictingCacheMap& other)
      : pairs(other.pairs),
        hashTable(other.hashTable.size()),
        hasher(other.hasher),
        capacity(other.capacity) {
    for (auto it = pairs.begin(); it != pairs.end(); ++it)
      Bucket(it->first).emplace_back(it);
  }

  EvictingCacheMap& operator=(const EvictingCacheMap& other) {
    if (this != &other) {
      pairs = other.pairs;
      hashTable.resize(other.hashTable.size());
      hasher = other.hasher;
      capacity = other.capacity;
      for (auto it = pairs.begin(); it != pairs.end(); ++it)
        Bucket(it->first).emplace_back(it);
    }
    return *this;
  }

  EvictingCacheMap(EvictingCacheMap&& other)
      : pairs(std::move(other.pairs)),
        hashTable(std::move(other.hashTable)),
        hasher(std::move(other.hasher)),
        capacity(other.capacity) {}

  EvictingCacheMap& operator=(EvictingCacheMap&& other) {
    if (this != &other) {
      pairs = std::move(other.pairs);
      hashTable = std::move(other.hashTable);
      hasher = std::move(other.hasher);
      capacity = other.capacity;
    }
    return *this;
  }

  ~EvictingCacheMap() = default;

  /**
   * Check for existence of a specific key in the map.  This operation has
   *     no effect on LRU order.
   * @param key key to search for
   * @return true if exists, false otherwise
   */
  bool exists(const TKey& key) const {
    const auto& bucket = Bucket(key);
    return std::find_if(bucket.begin(), bucket.end(), [&](iterator iter) {
             return iter->first == key;
           }) != bucket.end();
  }

  /**
   * Get the value associated with a specific key.  This function always
   *     promotes a found value to the head of the LRU.
   * @param key key associated with the value
   * @return the value if it exists
   */
  std::optional<TValue> get(const TKey& key) {
    auto iter = find(key);
    if (iter != pairs.end())
      return iter->second;
    else
      return {};
  }

  /**
   * Get the iterator associated with a specific key.  This function always
   *     promotes a found value to the head of the LRU.
   * @param key key to associate with value
   * @return the iterator of the object (a std::pair of const TKey, TValue) or
   *     end() if it does not exist
   */
  iterator find(const TKey& key) {
    const auto& bucket = Bucket(key);
    auto iter = std::find_if(bucket.begin(), bucket.end(),
                             [&](iterator iter) { return iter->first == key; });

    if (iter == bucket.end()) return pairs.end();

    pairs.splice(pairs.begin(), pairs, *iter);
    return pairs.begin();
  }

  /**
   * Erase the key-value pair associated with key if it exists.
   * @param key key associated with the value
   * @return true if the key existed and was erased, else false
   */
  bool erase(const TKey& key) {
    auto iter = find(key);
    if (iter == pairs.end()) return false;

    Bucket(key).remove_if([&](iterator iter) { return iter->first == key; });
    pairs.pop_front();
    return true;
  }

  /**
   * Set a key-value pair in the dictionary
   * @param key key to associate with value
   * @param value value to associate with the key
   */
  template <class T, class E>
  void put(T&& key, E&& value) {
    auto iter = find(key);

    if (iter != pairs.end()) {
      iter->second = std::forward<E>(value);
      return;
    }

    if (pairs.size() == capacity) {
      const auto& keyToRemove = pairs.back().first;
      Bucket(keyToRemove).remove_if([&](iterator iter) {
        return iter->first == keyToRemove;
      });
      pairs.pop_back();
    }

    pairs.emplace_front(std::forward<T>(key), std::forward<E>(value));
    if (LoadFactor() > MaxLoadFactor) Extend();
    Bucket(pairs.front().first).emplace_back(pairs.begin());
  }

  /**
   * Get the number of elements in the dictionary
   * @return the size of the dictionary
   */
  std::size_t size() const { return pairs.size(); }

  /**
   * Typical empty function
   * @return true if empty, false otherwise
   */
  bool empty() const { return pairs.size() == 0; }

  void clear() {
    pairs.clear();
    for (auto& list : hashTable) list.clear();
  }

  // Iterators and such
  iterator begin() noexcept { return pairs.begin(); }
  iterator end() noexcept { return pairs.end(); }
  const_iterator begin() const noexcept { return pairs.begin(); }
  const_iterator end() const noexcept { return pairs.end(); }
  const_iterator cbegin() const noexcept { return pairs.cbegin(); }
  const_iterator cend() const noexcept { return pairs.cend(); }

 private:
  void Extend() {
    std::size_t newHashTableSize = 2 * hashTable.size();
    if (static_cast<double>(newHashTableSize) * MaxLoadFactor > capacity)
      newHashTableSize = static_cast<std::size_t>(
          static_cast<double>(capacity) / MaxLoadFactor);
    hashTable.clear();
    hashTable.resize(newHashTableSize);

    for (auto it = pairs.begin(); it != pairs.end(); ++it)
      Bucket(it->first).emplace_back(it);
  }

  double LoadFactor() const {
    return static_cast<double>(pairs.size()) /
           static_cast<double>(hashTable.size());
  }
  const IteratorsList& Bucket(const TKey& key) const {
    return hashTable[hasher(key) % hashTable.size()];
  }
  IteratorsList& Bucket(const TKey& key) {
    return hashTable[hasher(key) % hashTable.size()];
  }

  constexpr static double MaxLoadFactor = 0.75;

  PairsList pairs;
  HashTable hashTable;
  THash hasher;
  size_t capacity;
};

#endif  // INCLUDE_EVICTINGCACHEMAP_H_

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