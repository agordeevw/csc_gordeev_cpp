#include "gtest/gtest.h"
#include "EvictingCacheMap.h"

using EvictingCacheMapii = EvictingCacheMap<int, int>;

TEST(EvictingCacheMap, CtorZeroCapacityThrow)
{
    EXPECT_THROW({EvictingCacheMapii map(0);}, std::logic_error);
}

TEST(EvictingCacheMap, CtorValidCapacity)
{
    EXPECT_NO_THROW({EvictingCacheMapii map(1);});
}

TEST(EvictingCacheMap, CtorInitialState)
{
    EvictingCacheMapii map(1);

    EXPECT_TRUE(map.size() == 0);
    EXPECT_TRUE(map.empty());
    EXPECT_TRUE(map.begin() == map.end());
    EXPECT_TRUE(map.cbegin() == map.cend());
}

TEST(EvictingCacheMap, CopyCtor)
{
    EvictingCacheMapii map(4);
    map.put(0, 1);
    map.put(2, 3);
    map.put(3, 4);
    map.put(5, 6);

    EvictingCacheMap mapcpy(map);
    int expected[] = {5, 6, 3, 4, 2, 3, 0, 1};

    int counter = 0;
    for (auto it = mapcpy.begin(); it != mapcpy.end(); ++it)
    {
        EXPECT_EQ(it->first, expected[counter++]);
        EXPECT_EQ(it->second, expected[counter++]);
    }
}

TEST(EvictingCacheMap, CopyAssign)
{
    EvictingCacheMapii map(4);
    map.put(0, 1);
    map.put(2, 3);
    map.put(3, 4);
    map.put(5, 6);

    EvictingCacheMap mapcpy = map;
    int expected[] = {5, 6, 3, 4, 2, 3, 0, 1};

    int counter = 0;
    for (auto it = mapcpy.begin(); it != mapcpy.end(); ++it)
    {
        EXPECT_EQ(it->first, expected[counter++]);
        EXPECT_EQ(it->second, expected[counter++]);
    }
}

TEST(EvictingCacheMap, MoveCtor)
{
    EvictingCacheMapii map(4);
    map.put(0, 1);
    map.put(2, 3);
    map.put(3, 4);
    map.put(5, 6);

    EvictingCacheMap mapcpy(std::move(map));
    int expected[] = {5, 6, 3, 4, 2, 3, 0, 1};

    int counter = 0;
    for (auto it = mapcpy.begin(); it != mapcpy.end(); ++it)
    {
        EXPECT_EQ(it->first, expected[counter++]);
        EXPECT_EQ(it->second, expected[counter++]);
    }
}

TEST(EvictingCacheMap, MoveAssign)
{
    EvictingCacheMapii map(4);
    map.put(0, 1);
    map.put(2, 3);
    map.put(3, 4);
    map.put(5, 6);

    EvictingCacheMap mapcpy = std::move(map);
    int expected[] = {5, 6, 3, 4, 2, 3, 0, 1};

    int counter = 0;
    for (auto it = mapcpy.begin(); it != mapcpy.end(); ++it)
    {
        EXPECT_EQ(it->first, expected[counter++]);
        EXPECT_EQ(it->second, expected[counter++]);
    }
}

TEST(EvictingCacheMap, PutWithoutEviction)
{
    EvictingCacheMapii map(4);
    for (int i = 0; i < 4; ++i)
    {
        ASSERT_FALSE(map.exists(i));
        map.put(i, i);
        ASSERT_TRUE(map.exists(i));
        ASSERT_TRUE(map.size() == size_t(i + 1));
    }

    for (int i = 0; i < 4; ++i)
    {
        map.put(i ,i);
        EXPECT_TRUE(map.size() == 4);
        EXPECT_TRUE(map.exists(i));
        EXPECT_EQ(map.begin()->first, i);
    }
}

TEST(EvictingCacheMap, Eviction)
{
    EvictingCacheMapii map(4);
    for (int i = 0; i < 4; ++i)
    {
        EXPECT_FALSE(map.exists(i));
        map.put(i, i);
        EXPECT_TRUE(map.exists(i));
        EXPECT_TRUE(map.size() == size_t(i + 1));
    }

    for (int i = 4; i < 8; ++i)
    {
        EXPECT_FALSE(map.exists(i));
        map.put(i, i);
        EXPECT_TRUE(map.exists(i));
        EXPECT_FALSE(map.exists(i - 4));
    }
}

TEST(EvictingCacheMap, GetMethod)
{
    EvictingCacheMapii map(4);
    for (int i = 0; i < 4; ++i)
        map.put(i, i);

    // (3, 3), (2, 2), (1, 1), (0, 0)

    for (int i = 0; i < 4; ++i)
    {
        auto opt = map.get(i);
        ASSERT_TRUE(opt.has_value());
        EXPECT_EQ(opt.value(), i);
        EXPECT_EQ(map.begin()->first, i);
    }

    auto opt = map.get(4);
    EXPECT_FALSE(opt.has_value());
}

TEST(EvictingCacheMap, FindMethod)
{
    EvictingCacheMapii map(4);
    for (int i = 0; i < 4; ++i)
        map.put(i, i);
    
    for (int i = 0; i < 4; ++i)
    {   
        auto it = map.find(i);
        EXPECT_TRUE(it == map.begin());
        EXPECT_TRUE(it != map.end());
    }
}

TEST(EvictingCacheMap, EraseMethod)
{
    EvictingCacheMapii map(4);
    for (int i = 0; i < 4; ++i)
        map.put(i, i);
    
    EXPECT_FALSE(map.erase(-1));

    for (int i = 0; i < 4; ++i)
    {
        EXPECT_TRUE(map.size() == static_cast<size_t>(4 - i));
        EXPECT_TRUE(map.erase(i));
        EXPECT_TRUE(map.size() == static_cast<size_t>(3 - i));
    }
}

TEST(EvictingCacheMap, ClearMethod)
{
    EvictingCacheMapii map(4);
    for (int i = 0; i < 4; ++i)
        map.put(i, i);
    
    map.clear();

    EXPECT_TRUE(map.size() == 0);
    EXPECT_TRUE(map.empty());
    EXPECT_TRUE(map.begin() == map.end());
    EXPECT_TRUE(map.cbegin() == map.cend());
}

TEST(EvictingCacheMap, Iterators)
{
    EvictingCacheMapii map(4);
    for (int i = 0; i < 4; ++i)
        map.put(i, i + 1);
    
    int expected[]{3, 4, 2, 3, 1, 2, 0, 1};

    int counter = 0;
    for (auto it = map.begin(); it != map.end(); ++it)
    {
        EXPECT_EQ(it->first, expected[counter++]);
        EXPECT_EQ(it->second,  expected[counter++]);
    }

    counter = 0;
    for (auto it = map.cbegin(); it != map.cend(); ++it)
    {
        EXPECT_EQ(it->first,  expected[counter++]);
        EXPECT_EQ(it->second,  expected[counter++]);
    }

    const auto& mapcref = map;

    counter = 0;
    for (auto it = mapcref.begin(); it != mapcref.end(); ++it)
    {
        EXPECT_EQ(it->first,  expected[counter++]);
        EXPECT_EQ(it->second,  expected[counter++]);
    }
}