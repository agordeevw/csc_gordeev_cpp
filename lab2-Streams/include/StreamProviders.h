#ifndef LAB2_STREAMS_INCLUDE_STREAMPROVIDERS_H_
#define LAB2_STREAMS_INCLUDE_STREAMPROVIDERS_H_

#include <optional>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace stream {

/*
  Empty streams are prohibited,
    do not hesitate to throw exceptions.
  Streams are not reusable.
    If operator or terminator was applied to stream,
    the initial stream is no longer accessible
    and StreamClosedException is thrown.
*/

class EmptyStreamException : public std::logic_error {
public:
  EmptyStreamException() :
    std::logic_error("Empty stream")
  {}
};

class StreamClosedException : public std::logic_error {
public:
  StreamClosedException() :
    std::logic_error("Stream closed")
  {}
};

namespace providers {

/*
  Provider class must have following interface:
  class Provider
  {
  public:
    bool Advance();
    auto& GetValue();
  }

  All provider traits for new provider must be defined,
    otherwise compilation will fail.

  bool Advance()
    Advances provider to the next element.
    Returns true if new element is successfully provided.
    If stream provider ended, returns false. In this case
      following calls to GetValue() are undefined.

  auto& GetValue()
    Returns current element of stream.
    Requires Advance() to be called at least once, otherwise
      behavior is undefined.
    Requires Advance() to be called after each call to GetValue(),
      otherwise behavior is undefined.
    If Advance() returned false, following calls of GetValue()
      are undefined.
*/

template <class Derived>
class CloseableProvider
{
public:
  CloseableProvider() {}
  CloseableProvider(CloseableProvider&& other) {
    other.Close();
  }
  bool IsClosed() { return closed; }
  void Close() { closed = true; }
private:
  bool closed = false;
};

template <class IteratorType>
class Iterator final : public CloseableProvider<Iterator<IteratorType>>
{
public:
  Iterator(IteratorType begin, IteratorType end) :
    current(begin),
    end(end)
  {}

  bool Advance() {
    if (first) {
      first = false;
      return current != end;
    }
    ++current;
    return current != end;
  }

  auto& GetValue() {
    return *current;
  }

private:
  bool first = true;
  IteratorType current;
  IteratorType end;
};

template <class GeneratorType>
class Generator final : public CloseableProvider<Generator<GeneratorType>>
{
  using value_type =
    std::invoke_result_t<GeneratorType>;

public:
  explicit Generator(GeneratorType&& generator) :
    generator(std::forward<GeneratorType>(generator))
  {}

  bool Advance() {
    current = generator();
    return true;
  }

  auto& GetValue() {
    return current.value();
  }

private:
  GeneratorType generator;
  std::optional<value_type> current;
};

template <class ContainerType>
class Container final : public CloseableProvider<Container<ContainerType>>
{
public:
  explicit Container(ContainerType&& container) :
    container(std::move(container)),
    provider(this->container.begin(), this->container.end())
  {}

  explicit Container(Container&& other) :
    container(std::move(other.container)),
    provider(this->container.begin(), this->container.end()),
    advanceCount(other.advanceCount)
  {
    for (size_t i = 0; i < advanceCount; ++i)
      provider.Advance();
  }

  bool Advance() {
    ++advanceCount;
    return provider.Advance();
  }

  auto& GetValue() {
    return provider.GetValue();
  }

private:
  using iterator_type = typename std::remove_const_t<
    std::remove_reference_t<ContainerType>>::iterator;

  ContainerType container;
  Iterator<iterator_type> provider;
  size_t advanceCount = 0;
};

template <class Provider>
class Get final : public CloseableProvider<Get<Provider>>
{
public:
  Get(Provider&& provider, size_t amount) :
    provider(std::move(provider)),
    amount(amount)
  {}

  bool Advance() {
    if (current < amount) {
      ++current;
      return provider.Advance();
    }
    return false;
  }

  auto& GetValue() {
    return provider.GetValue();
  }

private:
  Provider provider;
  size_t current = 0;
  size_t amount;
};

template <class Provider>
class Skip final : public CloseableProvider<Skip<Provider>>
{
public:
  Skip(Provider&& provider, size_t amount) :
    provider(std::move(provider)),
    amount(amount)
  {}

  bool Advance() {
    while (current < amount) {
      if (!provider.Advance())
        throw EmptyStreamException();
      ++current;
    }
    return provider.Advance();
  }

  auto& GetValue() {
    return provider.GetValue();
  }

private:
  Provider provider;
  size_t current = 0;
  size_t amount;
};

template <class Provider, class Transform>
class Map final : public CloseableProvider<Map<Provider, Transform>>
{
  using value_type = std::invoke_result_t<
    Transform,
    decltype(std::declval<Provider>().GetValue())
    >;

public:
  Map(Provider&& provider, Transform&& transform) :
    provider(std::move(provider)),
    transform(std::forward<Transform>(transform))
  {}

  bool Advance() {
    return provider.Advance();
  }

  auto& GetValue() {
    if constexpr (std::is_reference_v<value_type>) {
      return transform(provider.GetValue());
    } else {
      current = transform(provider.GetValue());
      return current.value();
    }
  }

private:
  Provider provider;
  Transform transform;
  std::conditional_t<
    std::is_reference_v<value_type>,
    int,  // unused type
    std::optional<value_type>
  > current;
};

template <class Provider, class Predicate>
class Filter final : public CloseableProvider<Filter<Provider, Predicate>>
{
public:
  Filter(Provider&& provider, Predicate&& predicate) :
    provider(std::move(provider)),
    predicate(std::forward<Predicate>(predicate))
  {}

  bool Advance() {
    while (provider.Advance()) {
      if (predicate(provider.GetValue()))
        return true;
    }
    return false;
  }

  auto& GetValue() {
    return provider.GetValue();
  }

private:
  Provider provider;
  Predicate predicate;
};

template <class Provider>
class Group final : public CloseableProvider<Group<Provider>>
{
  using value_type =
    std::vector<std::remove_reference_t<
      decltype(std::declval<Provider>().GetValue())>>;

public:
  Group(Provider&& provider, size_t size) :
    provider(std::move(provider)),
    size(size)
  {}

  bool Advance() {
    if (streamEnded)
      return false;
    current.clear();
    for (size_t i = 0; i < size; ++i) {
      if (provider.Advance()) {
        current.emplace_back(std::move(provider.GetValue()));
      } else {
        streamEnded = true;
        break;
      }
    }
    return true;
  }

  auto& GetValue() {
    return current;
  }

private:
  Provider provider;
  size_t size;
  value_type current;
  bool streamEnded = false;
};

namespace traits {

template <class Provider>
struct is_finite {};

template <class IteratorType>
struct is_finite<Iterator<IteratorType>> :
  std::true_type {};

template <class GeneratorType>
struct is_finite<Generator<GeneratorType>> :
  std::false_type {};

template <class ContainerType>
struct is_finite<Container<ContainerType>> :
  std::true_type {};

template <class Provider>
struct is_finite<Get<Provider>> :
  std::true_type {};

template <class Provider>
struct is_finite<Skip<Provider>> :
  is_finite<Provider> {};

template <class Provider, class Transform>
struct is_finite<Map<Provider, Transform>> :
  is_finite<Provider> {};

template <class Provider, class Predicate>
struct is_finite<Filter<Provider, Predicate>> :
  is_finite<Provider> {};

template <class Provider>
struct is_finite<Group<Provider>> :
  is_finite<Provider> {};

template <class Provider>
constexpr bool is_finite_v = is_finite<Provider>::value;

template <class T>
struct is_provider :
  std::false_type {};

template <class IteratorType>
struct is_provider<Iterator<IteratorType>> :
  std::true_type {};

template <class GeneratorType>
struct is_provider<Generator<GeneratorType>> :
  std::true_type {};

template <class ContainerType>
struct is_provider<Container<ContainerType>> :
  std::true_type {};

template <class Provider>
struct is_provider<Get<Provider>> :
  is_provider<Provider> {};

template <class Provider>
struct is_provider<Skip<Provider>> :
  is_provider<Provider> {};

template <class Provider, class Transform>
struct is_provider<Map<Provider, Transform>> :
  is_provider<Provider> {};

template <class Provider, class Predicate>
struct is_provider<Filter<Provider, Predicate>> :
  is_provider<Provider> {};

template <class Provider>
struct is_provider<Group<Provider>> :
  is_provider<Provider> {};

template <class Provider>
constexpr bool is_provider_v = is_provider<Provider>::value;

}  // namespace traits
}  // namespace providers
}  // namespace stream

#endif  // LAB2_STREAMS_INCLUDE_STREAMPROVIDERS_H_
