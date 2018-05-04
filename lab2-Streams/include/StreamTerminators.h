#ifndef INCLUDE_STREAMTERMINATORS_H
#define INCLUDE_STREAMTERMINATORS_H

#include <iostream>

namespace stream {
namespace terminators {

template <class IdentityFn, class Accumulator>
class Reduce
{
public:
  (IdentityFn&& identityFn, Accumulator&& accum) :
  identityFn(std::forward<IdentityFn>(identityFn)),
  accum(std::forward<Accumulator>(accum))
  {}

  template <class Provider>
  auto operator()(Stream<Provider>&& stream) {
    auto& provider = stream.GetProvider();
    if (!provider.Advance())
      throw EmptyStreamException();
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
  PrintTo(std::ostream& os, const char* delimiter = " ") : 
  os(os), delimiter(delimiter) {}

  template <class Provider>
  std::ostream& operator()(Stream<Provider>&& stream) {
    auto& provider = stream.GetProvider();

    if (!provider.Advance())
      throw EmptyStreamException();
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
        throw EmptyStreamException();
    return std::move(*provider.Get());
  }
};

} // namespace terminators
} // namespace stream

#endif
