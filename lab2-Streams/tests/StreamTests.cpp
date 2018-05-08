#include <algorithm>
#include <iterator>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>
#include "gtest/gtest.h"
#include "Stream.h"

using namespace stream;

class StreamTest : public ::testing::Test {
protected:
  std::vector<int> container = {1, 2, 3, 4, 5};
  std::vector<int> emptyContainer;

  class GeneratorClass {
  public:
    int operator()() { return counter++; }
  private:
    int counter = 1;
  };

  auto MakeStreamFromContainerIterators(std::vector<int>& cont) {
    return Stream(cont.begin(), cont.end());
  }

  auto MakeStreamFromLvalueContainer(std::vector<int>& cont) {
    return Stream(cont);
  }

  auto MakeStreamFromConstContainer(const std::vector<int>& cont) {
    return Stream(cont);
  }

  auto MakeStreamFromRvalueContainer(std::vector<int>&& cont) {
    return Stream(std::move(cont));
  }

  auto MakeStreamFromIlist() {
    return Stream{1, 2, 3, 4 ,5};
  }

  auto MakeStreamFromPack() {
    return Stream(1, 2, 3, 4, 5);
  }

  auto MakeStreamFromGenerator() {
    return Stream(GeneratorClass{}) | get(5);
  }

  template <class StreamGenerator>
  void RunEmptyStreamTests(StreamGenerator&& generator) {
    std::ostringstream oss;

    EXPECT_THROW(generator() | reduce(
      [](auto&& x, auto&& y) { return x; }),  EmptyStreamException);
    EXPECT_THROW(generator() | reduce(
      [](auto&& x) { return x; },
      [](auto&& x, auto&& y) { return x; }),  EmptyStreamException);
    EXPECT_THROW(generator() | sum(),         EmptyStreamException);
    EXPECT_THROW(generator() | print_to(oss), EmptyStreamException);
    EXPECT_THROW(generator() | to_vector(),   EmptyStreamException);
    EXPECT_THROW(generator() | nth(0),        EmptyStreamException);
  }

  template <class StreamGenerator>
  void RunStreamClosedStateCorrectnessTests(StreamGenerator&& generator) {
    auto stream = generator();
    ASSERT_FALSE(stream.GetProvider().IsClosed());
    ASSERT_FALSE(generator().GetProvider().IsClosed());

    auto compositeStream = generator() | get(10);
    ASSERT_FALSE(compositeStream.GetProvider().IsClosed());
    ASSERT_FALSE((generator() | get(10)).GetProvider().IsClosed());

    auto copiedStream = stream;
    ASSERT_FALSE(copiedStream.GetProvider().IsClosed());
    ASSERT_FALSE(stream.GetProvider().IsClosed());

    copiedStream | nth(0);
    ASSERT_TRUE(copiedStream.GetProvider().IsClosed());
    ASSERT_FALSE(stream.GetProvider().IsClosed());
    EXPECT_THROW(copiedStream | nth(0), StreamClosedException);

    auto movedToStream = std::move(stream);
    ASSERT_FALSE(movedToStream.GetProvider().IsClosed());
    ASSERT_TRUE(stream.GetProvider().IsClosed());
  }

  template <class StreamGenerator>
  void RunReduceTerminatorTests(StreamGenerator&& generator) {
    auto initialContainerSize = container.size();

    //  sum() terminator
    EXPECT_EQ(
      generator() | sum(),
      std::accumulate(container.begin(), container.end(), 0));
    ASSERT_EQ(container.size(), initialContainerSize);

    //  reduce with default id
    auto minValue =
      generator() | reduce([] (auto min, auto val) {
        return std::min(min, val); });

    EXPECT_EQ(minValue, *std::min_element(container.begin(), container.end()));
    ASSERT_EQ(container.size(), initialContainerSize);

    //  reduce with custom id
    std::string concatValuesString =
      generator() | reduce(
        [] (int x) { return std::to_string(x); },
        [] (std::string res, int val) { return res + std::to_string(val); });

    std::string expectedConcatString;
    for (auto&& val : container)
      expectedConcatString += std::to_string(val);

    EXPECT_EQ(concatValuesString, expectedConcatString);
    ASSERT_EQ(container.size(), initialContainerSize);

    //  reduce with effects
    std::vector<int> partialSums;
    generator() | reduce(
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

  template <class StreamGenerator>
  void RunPrintToTerminatorTests(StreamGenerator&& generator) {
    std::ostringstream oss;
    auto initialContainerSize = container.size();

    generator() | print_to(oss);
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

    generator() | print_to(oss, delim);
    ASSERT_EQ(initialContainerSize, container.size());

    it = container.begin();
    expectedOss << *it++;
    while (it != container.end())
      expectedOss << delim << *it++;

    EXPECT_EQ(oss.str(), expectedOss.str());
  }

  template <class StreamGenerator>
  void RunToVectorTerminatorTests(StreamGenerator&& generator) {
    auto initialContainerSize = container.size();

    auto vec = generator() | to_vector();
    ASSERT_EQ(container.size(), initialContainerSize);

    EXPECT_TRUE(std::equal(vec.begin(), vec.end(), container.begin()));
  }

  template <class StreamGenerator>
  void RunNthTerminatorTests(StreamGenerator&& generator) {
    auto initialContainerSize = container.size();

    for (size_t i = 0; i < container.size(); ++i) {
      auto value = generator() | nth(i);
      ASSERT_EQ(container.size(), initialContainerSize);
      EXPECT_EQ(value, container[i]);
    }

    EXPECT_THROW(generator() | nth(container.size()), EmptyStreamException);
  }

  template <class StreamGenerator>
  void RunGetOperatorTests(StreamGenerator&& generator) {
    EXPECT_THROW(generator() | get(0) | to_vector(), EmptyStreamException);

    const size_t toGetNumber = 2;
    auto partialGetVector = generator() | get(toGetNumber) | to_vector();

    EXPECT_TRUE(std::equal(
      partialGetVector.begin(), partialGetVector.end(), container.begin()));

    auto fullGetVector =
      generator() | get(container.size()) | to_vector();

    EXPECT_TRUE(std::equal(fullGetVector.begin(), fullGetVector.end(),
      container.begin()));

    auto oversizedGetVector = generator()
      | get(container.size() + 1) | to_vector();

    EXPECT_TRUE(std::equal(oversizedGetVector.begin(), oversizedGetVector.end(),
      container.begin()));
  }

  template <class StreamGenerator>
  void RunSkipOperatorTests(StreamGenerator&& generator) {
    size_t skipAmount = 2;
    auto vec = generator() | skip(skipAmount) | to_vector();

    auto containerIter = container.begin();
    std::advance(containerIter, skipAmount);
    EXPECT_TRUE(std::equal(vec.begin(), vec.end(), containerIter));

    size_t overboardSkipAmount = container.size();
    EXPECT_THROW(generator() | skip(overboardSkipAmount) | to_vector(),
      EmptyStreamException);
  }

  template <class StreamGenerator>
  void RunMapOperatorTests(StreamGenerator&& generator) {
    auto identityVec = generator()
      | map([](auto&& x) { return x; }) | to_vector();

    EXPECT_TRUE(std::equal(identityVec.begin(), identityVec.end(),
      container.begin()));

    std::ostringstream oss;
    generator()
      | map([](auto&& x) { return std::to_string(x); }) | print_to(oss);

    std::ostringstream expectedOss;
    auto it = container.begin();
    expectedOss << *it++;
    while (it != container.end())
      expectedOss << " " << *it++;

    EXPECT_EQ(oss.str(), expectedOss.str());
  }

  template <class StreamGenerator>
  void RunFilterOperatorTests(StreamGenerator&& generator) {
    auto acceptingFilterVec =
      generator() | filter([](auto&& x) { return true; }) | to_vector();

    EXPECT_TRUE(std::equal(acceptingFilterVec.begin(), acceptingFilterVec.end(),
      container.begin()));

    EXPECT_THROW(
      generator() | filter([](auto&& x) { return false; }) | to_vector(),
      EmptyStreamException);

    auto predicate = [](auto&& x) { return x % 2 == 0; };
    auto invPredicate = [&predicate](auto&& x) { return !predicate(x); };

    auto filteredVec =
      generator() | filter(predicate) | to_vector();

    std::vector<int> copyVec;
    std::remove_copy_if(container.begin(), container.end(),
      std::back_inserter(copyVec), invPredicate);

    EXPECT_TRUE(std::equal(filteredVec.begin(), filteredVec.end(),
      copyVec.begin()));
  }

  template <class StreamGenerator>
  void RunGroupOperatorTests(StreamGenerator&& generator) {
    std::size_t groupSize = 2;
    std::vector<std::vector<int>> groupedVec = {{1, 2}, {3, 4}, {5}};

    auto streamGroupedVec =
      generator() | group(groupSize) | to_vector();

    EXPECT_TRUE(std::equal(
      streamGroupedVec.begin(), streamGroupedVec.end(), groupedVec.begin(),
      [](auto&& lhs, auto&& rhs) {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin());
      }));
  }
};

#define RUN_STREAM_TESTING_METHOD(testing_method) \
  (testing_method)([this]{ \
    return MakeStreamFromContainerIterators(container); }); \
  (testing_method)([this]{ \
    return MakeStreamFromLvalueContainer(container); }); \
  (testing_method)([this]{ \
    return MakeStreamFromConstContainer(container); }); \
  (testing_method)([this]{ \
    return MakeStreamFromRvalueContainer(std::vector<int>(container)); }); \
  (testing_method)([this]{ \
    return MakeStreamFromIlist(); }); \
  (testing_method)([this]{ \
    return MakeStreamFromPack(); }); \
  (testing_method)([this]{ \
    return MakeStreamFromGenerator(); });


TEST_F(StreamTest, EmptyStreamInitialization)
{
  RunEmptyStreamTests([this]{
    return MakeStreamFromContainerIterators(emptyContainer); });
  RunEmptyStreamTests([this]{
    return MakeStreamFromLvalueContainer(emptyContainer); });
  RunEmptyStreamTests([this]{
    return MakeStreamFromConstContainer(emptyContainer); });
  RunEmptyStreamTests([this]{
    return MakeStreamFromRvalueContainer(std::vector<int>{}); });
}

TEST_F(StreamTest, StreamClosedStateCorrectness)
{
  RUN_STREAM_TESTING_METHOD(RunStreamClosedStateCorrectnessTests);
}

TEST_F(StreamTest, ReduceTerminator)
{
  RUN_STREAM_TESTING_METHOD(RunReduceTerminatorTests);
}

TEST_F(StreamTest, PrintToTerminator)
{
  RUN_STREAM_TESTING_METHOD(RunPrintToTerminatorTests);
}

TEST_F(StreamTest, ToVectorTerminator)
{
  RUN_STREAM_TESTING_METHOD(RunToVectorTerminatorTests);
}

TEST_F(StreamTest, NthTerminator)
{
  RUN_STREAM_TESTING_METHOD(RunNthTerminatorTests);
}

TEST_F(StreamTest, GetOperator)
{
  RUN_STREAM_TESTING_METHOD(RunGetOperatorTests);
}

TEST_F(StreamTest, SkipOperator)
{
  RUN_STREAM_TESTING_METHOD(RunSkipOperatorTests);
}

TEST_F(StreamTest, MapOperator)
{
  RUN_STREAM_TESTING_METHOD(RunMapOperatorTests);
}

TEST_F(StreamTest, FilterOperator)
{
  RUN_STREAM_TESTING_METHOD(RunFilterOperatorTests);
}

TEST_F(StreamTest, GroupOperator)
{
  RUN_STREAM_TESTING_METHOD(RunGroupOperatorTests);
}
