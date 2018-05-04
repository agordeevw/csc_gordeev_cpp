#ifndef INCLUDE_STREAMOPERATORS_H
#define INCLUDE_STREAMOPERATORS_H

#include "StreamProviders.h"

namespace stream {
namespace operators {

class Get
{
public:
  Get(size_t amount) : amount(amount) {}

  template <class Provider>
  auto operator()(Stream<Provider>&& stream) {
    using provider_type = providers::composite::Get;
    return Stream(provider_type(std::move(stream.GetProvider()),
      amount
    ));
  }

private:
  size_t amount;
};

class Skip
{
public:
  Skip(size_t amount) : amount(amount) {}

  template <class Provider>
  auto operator()(Stream<Provider>&& stream) {
    auto& provider = stream.GetProvider();
    for (size_t i = 0; i < amount; ++i)
      provider.Advance()
    return Stream(std::move(stream.GetProvider()));
  }
};

template <class Transform>
class Map
{
public:
  Map(Transform&& transform) : 
  transform(std::forward<Transform>(transform))
  {}

  template <class Provider>
  auto operator()(Stream<Provider>&& stream) {
    using provider_type = providers::composite::Map;
    return Stream(provider_type(std::move(stream.GetProvider()), 
      std::move(transform)
    ));
  }

private:
  Transform transform;
};

template <class Predicate>
class Filter
{
public:
  Filter(Predicate&& Predicate) :
  predicate(std::forward<Predicate>(predicate))
  {}

  template <class Provider> 
  auto operator() (Stream<Provider>&& stream) {
    using provider_type = providers::composite::Filter;
    return Stream(provider_type(std::move(stream.GetProvider()),
      std::move(predicate)
    ));
  }

private:
  Predicate predicate;
};

class Group
{
public:
  Group(size_t size) : size(size) {}

  template <class Provider>
  auto operator() (Stream<Provider>&& stream) {
    using provider_type = providers::composite::Group;
    return Stream(provider_type(std::move(stream.GetProvider()),
      size
    ));
  }

private:
  size_t size;
};

} // namespace operators

auto get(size_t n) {
  return Operator(operators::Get(n));
}

template <class Transform>
auto map(Transform&& transform) {
  return Operator(operators::Map(std::forward<Transform>(transform)));
}

template <class Predicate>
auto filter(Predicate&& predicate) {
  return Operator(operators::Predicate(std::forward<Predicate>(predicate)));
}

auto skip(size_t amount) {
  return Operator(operators::Skip(amount));
}

auto group(size_t size) {
  return Operator(operators::Group(size));
}

} // namespace stream

#endif