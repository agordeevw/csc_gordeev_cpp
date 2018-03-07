#ifndef INCLUDE_EVICTINGCACHEMAP_H_
#define INCLUDE_EVICTINGCACHEMAP_H_

#include <cstddef>
#include <functional>
#include <optional>
#include <stdexcept>

template <class TKey, class TValue, class THash = std::hash<TKey>>
class EvictingCacheMap final {
  using PairsList = std::list<std::pair<const TKey, TValue>>;
  using IteratorsList =
      std::list<typename PairsList::iterator> using HashTable =
          std::vector<IteratorsList>;

 public:
  using iterator = typename PairsList::iterator;
  using const_iterator = typename PairsList::const_iterator;
  /**
   * Construct a EvictingCacheMap
   * @param capacity maximum size of the cache map.  Once the map size exceeds
   *    maxSize, the map will begin to evict.
   */
  explicit EvictingCacheMap(std::size_t capacity)
      : pairs_end(pairs.end()), size(0), capacity(capacity) {
    if (capacity == 0)
      throw std::logic_error("Unable to create cache of size 0");

    const std::size_t initialTableSize = 4;
    hashTable.resize(initialTableSize);
  }

  EvictingCacheMap(const EvictingCacheMap& other)
      : pairs(other.pairs),
        pairs_end(pairs.end()),
        hashTable(other.hashTable.size()),
        hasher(other.hasher),
        capacity(other.capacity) {
    for (auto it = pairs.begin(); it != pairs.end(); ++it)
      Bucket(it->first).emplace_back(it);
  }

  EvictingCacheMap& operator=(const EvictingCacheMap& other) {
    if (this != &other) {
      pairs = other.pairs;
      pairs_end = pairs.end();
      hashTable.resize(other.hashTable.size());
      hasher = other.hasher;
      capacity(other.capacity);
      for (auto it = pairs.begin(); it != pairs.end(); ++it)
        Bucket(it->first).emplace_back(it);
    }
    return *this;
  }

  EvictingCacheMap(EvictingCacheMap&& other)
      : pairs(std::move(other.pairs)),
        pairs_end(pairs.end()),
        hashTable(std::move(other.hashTable)),
        hasher(std::move(other.hasher)),
        capacity(other.capacity) {}

  EvictingCacheMap& operator=(EvictingCacheMap&& other) {
    if (this != &other) {
      pairs = std::move(other.pairs);
      pairs_end = pairs.end();
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
  bool exists(const TKey& key) const;

  /**
   * Get the value associated with a specific key.  This function always
   *     promotes a found value to the head of the LRU.
   * @param key key associated with the value
   * @return the value if it exists
   */
  std::optional<TValue> get(const TKey& key);

  /**
   * Get the iterator associated with a specific key.  This function always
   *     promotes a found value to the head of the LRU.
   * @param key key to associate with value
   * @return the iterator of the object (a std::pair of const TKey, TValue) or
   *     end() if it does not exist
   */
  iterator find(const TKey& key);

  /**
   * Erase the key-value pair associated with key if it exists.
   * @param key key associated with the value
   * @return true if the key existed and was erased, else false
   */
  bool erase(const TKey& key);

  /**
   * Set a key-value pair in the dictionary
   * @param key key to associate with value
   * @param value value to associate with the key
   */
  template <class T, class E>
  void put(T&& key, E&& value);

  /**
   * Get the number of elements in the dictionary
   * @return the size of the dictionary
   */
  std::size_t size() const;

  /**
   * Typical empty function
   * @return true if empty, false otherwise
   */
  bool empty() const;

  void clear();

  // Iterators and such
  iterator begin() noexcept;
  iterator end() noexcept;
  const_iterator begin() const noexcept;
  const_iterator end() const noexcept;
  const_iterator cbegin() const noexcept;
  const_iterator cend() const noexcept;

 private:
  void extend();

  constexpr double MaxLoadFactor() = 0.75;
  inline double LoadFactor() const {
    return static_cast<double>(size) / static_cast<double>(table_size);
  }
  inline IteratorsList& Bucket(const TKey& key) {
    return hashTable[hasher(key) % hashTable.size()];
  }

  PairsList pairs;
  iterator pairs_end;
  HashTable hashTable;
  THash hasher;

  size_t capacity;
};

#endif  // INCLUDE_EVICTINGCACHEMAP_H_
