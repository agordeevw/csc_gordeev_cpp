#ifndef LAB2_STREAMS_INCLUDE_STREAMTERMINATORS_H_
#define LAB2_STREAMS_INCLUDE_STREAMTERMINATORS_H_

#include <iostream>
#include <utility>
#include <vector>

#include "StreamProviders.h"

namespace stream {

template <class> class Stream;
template <class, class> class Compose;

namespace terminators {

/*
  Terminator class must be Callable:
    template <class Provider>
    auto operator()(Stream<Provider>&&) -> ...

  All terminator traits for new terminator must be implemented,
    otherwise compilation will fail.

  If stream is empty,
    EmptyStreamException must be thrown.
*/

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
      throw EmptyStreamException();
    auto result(identityFn(provider.GetValue()));
    while (provider.Advance())
      result = accum(result, provider.GetValue());
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
    os(os),
    delimiter(delimiter)
  {}

  template <class Provider>
  std::ostream& operator()(Stream<Provider>&& stream) {
    auto& provider = stream.GetProvider();
    if (!provider.Advance())
      throw EmptyStreamException();
    os << provider.GetValue();
    while (provider.Advance())
      os << delimiter << provider.GetValue();
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
    using value_type =
      std::remove_const_t<std::remove_reference_t<
        decltype(std::declval<Provider>().GetValue())>>;

    auto& provider = stream.GetProvider();
    std::vector<value_type> result;
    if (!provider.Advance())
      throw EmptyStreamException();
    do {
      result.emplace_back(std::move(provider.GetValue()));
    } while (provider.Advance());
    return result;
  }
};

class Nth
{
public:
  explicit Nth(size_t index) :
    index(index)
  {}

  template <class Provider>
  auto operator()(Stream<Provider>&& stream) {
    auto& provider = stream.GetProvider();
    for (size_t i = 0; i <= index; ++i)
      if (!provider.Advance())
        throw EmptyStreamException();
    return provider.GetValue();
  }

private:
  size_t index;
};

namespace traits
{
template <class>
struct supports_infinite {};

template <class I, class A>
struct supports_infinite<Reduce<I, A>> :
  std::false_type {};

template <>
struct supports_infinite<PrintTo> :
  std::false_type {};

template <>
struct supports_infinite<ToVector>:
  std::false_type {};

template <>
struct supports_infinite<Nth>:
  std::true_type {};

template <class Term, class Op>
struct supports_infinite<Compose<Term, Op>> :
  supports_infinite<Term> {};

template <class Term>
constexpr bool supports_infinite_v = supports_infinite<Term>::value;

}  // namespace traits
}  // namespace terminators
}  // namespace stream

#endif  // LAB2_STREAMS_INCLUDE_STREAMTERMINATORS_H_
