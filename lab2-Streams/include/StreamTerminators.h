#ifndef INCLUDE_STREAMTERMINATORS_H
#define INCLUDE_STREAMTERMINATORS_H

#include <iostream>

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

template <class IdentityFn, class Accumulator, bool tryOptimized = false>
class Reduce
{

template <class Provider>
using identity_value_type =
  std::invoke_result_t<
    IdentityFn,
    decltype(std::declval<Provider>().GetValue())
    >;

template <class Provider>
using accum_value_type = 
  std::invoke_result_t<
    Accumulator,
    identity_value_type<Provider>,
    decltype(std::declval<Provider>().GetValue())
    >;

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

    if constexpr (tryOptimized 
      && std::is_reference_v<accum_value_type<Provider>>) {
        return OptimizedReduce(provider);
      } else {
        return UnoptimizedReduce(provider);
      }
  }

private:
  template <class Provider>
  auto UnoptimizedReduce(Provider& provider) {
    auto result(identityFn(provider.GetValue()));
    while (provider.Advance())
      result = accum(result, provider.GetValue());
    return result;
  }

  template <class Provider>
  auto OptimizedReduce(Provider& provider) {
    using identity_value_type_unref = 
      std::remove_reference_t<identity_value_type<Provider>>;

    identity_value_type_unref* result;
    std::optional<identity_value_type_unref> temp;

    if constexpr (std::is_reference_v<identity_value_type<Provider>>) {
      result = &identityFn(provider.GetValue());
    } else {
      temp = identityFn(provider.GetValue());
      result = &temp;
    } 

    while (provider.Advance())
      result = &accum(*result, provider.GetValue());

    if constexpr (std::is_const_v<decltype(*result)>)
      return *result;
    else
      return std::move(*result);
  }

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
      std::remove_reference_t<decltype(std::declval<Provider>().GetValue())>;

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
  Nth(size_t index) :
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

  template <class I, class A, bool b>
  struct supports_infinite<Reduce<I, A, b>> :
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

} // namespace traits
} // namespace terminators
} // namespace stream

#endif
