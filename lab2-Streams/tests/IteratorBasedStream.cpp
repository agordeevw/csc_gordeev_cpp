#include <algorithm>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>
#include "gtest/gtest.h"
#include "Stream.h"

using namespace stream;

TEST(IteratorBasedStream, EmptyStream)
{
  std::ostringstream oss;
  std::vector<int> emptyContainer;

  EXPECT_THROW(Stream(emptyContainer.begin(), emptyContainer.end())
    | reduce(
      [](auto&& x, auto&& y) { return x; }
      ), EmptyStreamException);
  EXPECT_THROW(Stream(emptyContainer.begin(), emptyContainer.end())
    | reduce(
      [](auto&& x) { return x; },
      [](auto&& x, auto&& y) { return x; }
      ), EmptyStreamException);
  EXPECT_THROW(Stream(emptyContainer.begin(), emptyContainer.end())
    | sum(),
    EmptyStreamException);
  EXPECT_THROW(Stream(emptyContainer.begin(), emptyContainer.end())
    | print_to(oss),
    EmptyStreamException);
  EXPECT_THROW(Stream(emptyContainer.begin(), emptyContainer.end())
    | to_vector(),
    EmptyStreamException);
  EXPECT_THROW(Stream(emptyContainer.begin(), emptyContainer.end())
    | nth(0),
    EmptyStreamException);
}

TEST(IteratorBasedStream, ClosedStreamBehavior)
{
  std::vector<int> container{1, 2, 3, 4, 5};
  Stream stream(container.begin(), container.end());
  ASSERT_FALSE(stream.GetProvider().IsClosed());
  stream | sum();
  ASSERT_TRUE(stream.GetProvider().IsClosed());
  EXPECT_THROW(stream | sum(), StreamClosedException);

  Stream otherStream(container.begin(), container.end());
  ASSERT_FALSE(otherStream.GetProvider().IsClosed());
  auto newStream = otherStream | get(10);
  ASSERT_FALSE(newStream.GetProvider().IsClosed());
  ASSERT_TRUE(otherStream.GetProvider().IsClosed());
}

TEST(IteratorBasedStream, ReduceTerminator)
{
  std::vector<int> container{1, 2, 3, 4, 5};
  auto initialContainerSize = container.size();

  //  sum() terminator
  EXPECT_EQ(Stream(container.begin(), container.end()) | sum(),
    std::accumulate(container.begin(), container.end(), 0));
  ASSERT_EQ(container.size(), initialContainerSize);

  //  reduce with default id
  auto minValue =
    Stream(container.begin(), container.end())
    | reduce([] (auto min, auto val) {
        return std::min(min, val); });

  EXPECT_EQ(minValue,
    *std::min_element(container.begin(), container.end()));
  ASSERT_EQ(container.size(), initialContainerSize);

  //  reduce with custom id
  std::string concatValuesString =
    Stream(container.begin(), container.end())
    | reduce(
        [] (int x) { return std::to_string(x); },
        [] (std::string res, int val) { return res + std::to_string(val); });

  std::string expectedConcatString;
  for (auto&& val : container) {
    expectedConcatString += std::to_string(val);
  }

  EXPECT_EQ(concatValuesString, expectedConcatString);
  ASSERT_EQ(container.size(), initialContainerSize);

  //  reduce with external modification
  std::vector<int> partialSums;
  Stream(container.begin(), container.end())
  | reduce(
    [&partialSums](int x) {
      partialSums.emplace_back(x);
      return x; },
    [&partialSums](int x, int y) {
      partialSums.emplace_back(x + y);
      return x + y;
    });
  ASSERT_EQ(container.size(), initialContainerSize);
  EXPECT_EQ(partialSums.size(), container.size());

  std::vector<int> expectedPartialSums;
  int sum = 0;
  for (auto val : container) {
    sum += val;
    expectedPartialSums.emplace_back(sum);
  }

  EXPECT_TRUE(std::equal(partialSums.begin(), partialSums.end(),
    expectedPartialSums.begin()));
}

TEST(IteratorBasedStream, PrintToTerminator)
{
  std::ostringstream oss;
  std::vector<int> container{1, 2, 3, 4, 5};
  auto initialContainerSize = container.size();

  Stream(container.begin(), container.end()) | print_to(oss);
  ASSERT_EQ(initialContainerSize, container.size());

  std::ostringstream expectedOss;
  auto it = container.begin();
  expectedOss << *it++;
  while (it != container.end())
    expectedOss << " " << *it++;

  EXPECT_EQ(oss.str(), expectedOss.str());

  oss.str("");
  expectedOss.str("");
  const char* delim = "-";

  Stream(container.begin(), container.end()) | print_to(oss, delim);
  ASSERT_EQ(initialContainerSize, container.size());

  it = container.begin();
  expectedOss << *it++;
  while (it != container.end())
    expectedOss << delim << *it++;

  EXPECT_EQ(oss.str(), expectedOss.str());
}

TEST(IteratorBasedStream, ToVectorTerminator)
{
  std::vector<int> container{1, 2, 3, 4, 5};
  auto initialContainerSize = container.size();

  auto vec =
    Stream(container.begin(), container.end())
    | to_vector();
  ASSERT_EQ(container.size(), initialContainerSize);

  EXPECT_TRUE(std::equal(vec.begin(), vec.end(), container.begin()));
}

TEST(IteratorBasedStream, NthTerminator)
{
  std::vector<int> container{1, 2, 3, 4, 5};
  auto initialContainerSize = container.size();

  for (size_t i = 0; i < container.size(); ++i) {
    auto value =
      Stream(container.begin(), container.end())
      | nth(i);
    ASSERT_EQ(container.size(), initialContainerSize);
    EXPECT_EQ(value, container[i]);
  }

  EXPECT_THROW(
    Stream(container.begin(), container.end()) | nth(container.size()),
    EmptyStreamException);
}

TEST(IteratorBasedStream, GetOperator)
{
  std::vector<int> container{1, 2, 3, 4, 5};

  EXPECT_THROW(Stream(container.begin(), container.end())
    | get(0) | to_vector(), EmptyStreamException);

  const size_t toGetNumber = 2;
  auto partialGetVector =
    Stream(container.begin(), container.end())
    | get(toGetNumber) | to_vector();

  EXPECT_TRUE(std::equal(
    partialGetVector.begin(), partialGetVector.end(), container.begin()));

  auto fullGetVector =
    Stream(container.begin(), container.end())
    | get(container.size()) | to_vector();

  EXPECT_TRUE(std::equal(
    fullGetVector.begin(), fullGetVector.end(), container.begin()));

  auto oversizedGetVector =
    Stream(container.begin(), container.end())
    | get(container.size() + 1) | to_vector();

  EXPECT_TRUE(std::equal(
    oversizedGetVector.begin(), oversizedGetVector.end(), container.begin()));
}

TEST(IteratorBasedStream, SkipOperator)
{
  std::vector<int> container{1, 2, 3, 4, 5};

  size_t skipAmount = 2;
  auto vec =
    Stream(container.begin(), container.end())
    | skip(skipAmount) | to_vector();

  auto containerIter = container.begin();
  std::advance(containerIter, skipAmount);
  EXPECT_TRUE(std::equal(vec.begin(), vec.end(),
    containerIter));

  size_t overboardSkipAmount = container.size();
  EXPECT_THROW(Stream(container.begin(), container.end())
    | skip(overboardSkipAmount) | to_vector(),
    EmptyStreamException);
}

TEST(IteratorBasedStream, MapOperator)
{
  std::vector<int> container{1, 2, 3, 4, 5};

  auto identityVec =
    Stream(container.begin(), container.end())
    | map([](auto&& x) { return x; })
    | to_vector();

  EXPECT_TRUE(std::equal(identityVec.begin(), identityVec.end(),
    container.begin()));

  std::ostringstream oss;
  Stream(container.begin(), container.end())
  | map([](auto&& x) { return std::to_string(x); })
  | print_to(oss);

  std::ostringstream expectedOss;
  auto it = container.begin();
  expectedOss << *it++;
  while (it != container.end())
    expectedOss << " " << *it++;

  EXPECT_EQ(oss.str(), expectedOss.str());
}

TEST(IteratorBasedStream, FilterOperator)
{
  std::vector<int> container{1, 2, 3, 4, 5};

  auto acceptingFilterVec =
    Stream(container.begin(), container.end())
    | filter([](auto&& x) { return true; })
    | to_vector();

  EXPECT_TRUE(std::equal(acceptingFilterVec.begin(), acceptingFilterVec.end(),
    container.begin()));

  EXPECT_THROW(Stream(container.begin(), container.end())
    | filter([](auto&& x) { return false; })
    | to_vector(),
    EmptyStreamException);

  auto predicate = [](auto&& x) { return x % 2 == 0; };
  auto invPredicate = [&predicate](auto&& x) { return !predicate(x); };

  auto filteredVec =
    Stream(container.begin(), container.end())
    | filter(predicate)
    | to_vector();

  std::vector<int> copyVec;
  std::remove_copy_if(container.begin(), container.end(),
    std::back_inserter(copyVec), invPredicate);

  EXPECT_TRUE(std::equal(filteredVec.begin(), filteredVec.end(),
    copyVec.begin()));
}

TEST(IteratorBasedStream, GroupOperator)
{
  std::vector<int> container{1, 2, 3, 4, 5};
  std::size_t groupSize = 2;
  std::vector<std::vector<int>> groupedVec = {{1, 2}, {3, 4}, {5}};

  auto streamGroupedVec =
    Stream(container.begin(), container.end())
    | group(groupSize) | to_vector();

  EXPECT_TRUE(std::equal(
    streamGroupedVec.begin(),
    streamGroupedVec.end(),
    groupedVec.begin(),
    [](auto&& lhs, auto&& rhs) {
      return std::equal(lhs.begin(), lhs.end(), rhs.begin());
    }));
}
