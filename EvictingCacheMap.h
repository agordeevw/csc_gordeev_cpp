// Copyright 2018 Alexander Gordeev

#ifndef EVICTINGCACHEMAP_H_
#define EVICTINGCACHEMAP_H_

template <class TKey, class TValue, class THash = std::hash<TKey>>
class EvictingCacheMap final {
 public:
    /**
     * Construct a EvictingCacheMap
     * @param capacity maximum size of the cache map.  Once the map size exceeds
     *    maxSize, the map will begin to evict.
    */
    explicit EvictingCacheMap(std::size_t capacity);

    EvictingCacheMap(const EvictingCacheMap&);

    EvictingCacheMap& operator=(const EvictingCacheMap&);

    EvictingCacheMap(EvictingCacheMap&&);

    EvictingCacheMap& operator=(EvictingCacheMap&&);

    ~EvictingCacheMap();

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
};

#endif  // EVICTINGCACHEMAP_H_
