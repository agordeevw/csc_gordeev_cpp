#ifndef INCLUDE_STREAMTERMINATORS_H
#define INCLUDE_STREAMTERMINATORS_H

#include <iostream>

#include "StreamProviders.h"

namespace stream {
namespace terminators {

template <class IdentityFn, class Accumulator>
class Reduce
{
public:
  Reduce(IdentityFn&& identityFn, Accumulator&& accum) :
  identityFn(std::forward<IdentityFn>(identityFn)),
  accum(std::forward<Accumulator>(accum))
  {}

  template <class Provider>
  auto operator()(Stream<Provider>&& stream) {
    auto& provider = stream.GetProvider();
    if (!provider.Advance())
      throw providers::EmptyStreamException();
    auto result = identityFn(std::move(*provider.Get()));
    while (provider.Advance())
      result = accum(result, std::move(*provider.Get()));
    return result;
  }

private:
  IdentityFn identityFn;
  Accumulator accum;
};

class PrintTo
{
public:
  PrintTo(std::ostream& os, const char* delimiter) : 
  os(os), delimiter(delimiter) {}

  template <class Provider>
  std::ostream& operator()(Stream<Provider>&& stream) {
    auto& provider = stream.GetProvider();

    if (!provider.Advance())
      throw providers::EmptyStreamException();
    os << std::move(*provider.Get());
    while (provider.Advance())
      os << delimiter << std::move(*provider.Get());
    return os;
  }

private:
  std::ostream& os;
  const char* delimiter;
};

class ToVector
{
public:
  ToVector() {}

  template <class Provider>
  auto operator()(Stream<Provider>&& stream) {
    auto& provider = stream.GetProvider();
    std::vector<typename Provider::value_type> result;
    while (provider.Advance())
      result.emplace_back(std::move(*provider.Get()));
    return result;
  }
};

class Nth
{
public:
  Nth(size_t index) : index(index) {}

  template <class Provider>
  auto operator()(Stream<Provider>&& stream) {
    auto& provider = stream.GetProvider();
    for (size_t i = 0; i <= index; ++i)
      if (!provider.Advance())
        throw providers::EmptyStreamException();
    return std::move(*provider.Get());
  }

private:
  size_t index;
};

} // namespace terminators

template <class Accumulator>
auto reduce(Accumulator&& accum) {
  return Terminator(terminators::Reduce(
    [](auto x) { return x; },
    std::forward<Accumulator>(accum)
  ));
}

template <class IdentityFn, class Accumulator>
auto reduce(IdentityFn&& identityFn, Accumulator&& accum) {
  return Terminator(terminators::Reduce(
    std::forward<IdentityFn>(identityFn),
    std::forward<Accumulator>(accum)
  ));
}

auto sum() {
  return Terminator(terminators::Reduce(
    [](auto x) { return x; },
    [](auto x, auto y) { return x + y; }
  ));
}

auto print_to(std::ostream& os, const char* delimiter = " ") {
  return Terminator(terminators::PrintTo(
    os, delimiter
  ));
}

auto to_vector() {
  return Terminator(terminators::ToVector());
}

auto nth(size_t index) {
  return Terminator(terminators::Nth(index));
}

} // namespace stream

#endif
