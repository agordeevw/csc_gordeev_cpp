#ifndef LAB2_STREAMS_INCLUDE_STREAMOPERATORS_H_
#define LAB2_STREAMS_INCLUDE_STREAMOPERATORS_H_

#include <utility>

#include "StreamProviders.h"

namespace stream {

template <class> class Stream;

namespace operators {

/*
  Operator class must be Callable:
    template <class Provider>
    auto operator()(Stream<Provider>&&) -> Stream<...>

  Operators __ARE NOT ALLOWED AT ANY CIRCUMSTANCES__
    to perform any operations with providers
    (e.g. use provider.GetValue() and provider.Advance())
    to enforce the rule that no calculations happen
    before stream is terminated.

  Information:
    Attentive reader might notice that all operator classes
    can be easily substituted with lambdas. But named operators
    have their benefits, for example, possibility of traits implementation.
    Look at terminators, for example.
*/

class Get
{
public:
  explicit Get(std::size_t amount) :
    amount(amount)
  {}

  template <class Provider>
  auto operator()(Stream<Provider>&& stream) {
    return Stream(
      providers::Get<Provider>(
        std::move(stream.GetProvider()),
        amount));
  }

private:
  std::size_t amount;
};

class Skip
{
public:
  explicit Skip(size_t amount) :
    amount(amount)
  {}

  template <class Provider>
  auto operator()(Stream<Provider>&& stream) {
    return Stream(
      providers::Skip<Provider>(
        std::move(stream.GetProvider()),
        amount));
  }

private:
  size_t amount;
};

template <class Transform>
class Map
{
public:
  explicit Map(Transform&& transform) :
    transform(std::forward<Transform>(transform))
  {}

  template <class Provider>
  auto operator()(Stream<Provider>&& stream) {
    return Stream(
      providers::Map<Provider, Transform>(
        std::move(stream.GetProvider()),
        std::forward<Transform>(transform)));
  }

private:
  Transform transform;
};

template <class Predicate>
class Filter
{
public:
  explicit Filter(Predicate&& predicate) :
    predicate(std::forward<Predicate>(predicate))
  {}

  template <class Provider>
  auto operator() (Stream<Provider>&& stream) {
    return Stream(
      providers::Filter<Provider, Predicate>(
        std::move(stream.GetProvider()),
        std::forward<Predicate>(predicate)));
  }

private:
  Predicate predicate;
};

class Group
{
public:
  explicit Group(size_t size) :
    size(size)
  {}

  template <class Provider>
  auto operator() (Stream<Provider>&& stream) {
    return Stream(
      providers::Group<Provider>(
        std::move(stream.GetProvider()),
        size));
  }

private:
  size_t size;
};

}  // namespace operators
}  // namespace stream

#endif  // LAB2_STREAMS_INCLUDE_STREAMOPERATORS_H_
