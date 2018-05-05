#ifndef INCLUDE_STREAMPROVIDERS_H
#define INCLUDE_STREAMPROVIDERS_H

#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace stream {

class EmptyStreamException : public std::logic_error
{
public:
  EmptyStreamException() : 
  std::logic_error("Empty stream") {};
};

namespace providers {

template <class IteratorType>
class Iterator final
{
public:
  using value_type = typename IteratorType::value_type;

  Iterator(IteratorType begin, IteratorType end) :
  current(begin), end(end)
  {}

  bool Advance() {
    if (first) {
      first = false;
      return current != end;
    }
    ++current;
    return current != end;
  }

  std::shared_ptr<value_type> GetValue() {
    return std::make_shared<value_type>(std::move(*current));
  }

private:
  bool first = true;
  IteratorType current;
  IteratorType end;
};

template <class GeneratorType>
class Generator final
{
public:
  using value_type = std::invoke_result_t<GeneratorType>;

  Generator(GeneratorType&& generator) :
  generator(std::forward<GeneratorType>(generator))
  {}

  bool Advance() {
    current = std::make_shared<value_type>(generator());
    return true;
  }

  std::shared_ptr<value_type> GetValue() {
    return current;
  }

private:
  GeneratorType generator;
  std::shared_ptr<value_type> current;
};

template <class ContainerType>
class Container final
{
public:
  using BaseContainerType = std::remove_reference_t<ContainerType>;
  using value_type = typename BaseContainerType::value_type;

  Container(const BaseContainerType& container) :
  container(container),
  provider(this->container.begin(), this->container.end())
  {}

  Container(ContainerType&& container) :
  container(std::forward<BaseContainerType>(container)),
  provider(this->container.begin(), this->container.end())
  {}

  Container(Container&& other) :
  container(std::move(other.container)),
  provider(this->container.begin(), this->container.end())
  {}

  bool Advance() {
    return provider.Advance();
  }

  std::shared_ptr<value_type> GetValue() {
    return provider.GetValue();
  }

private:
  BaseContainerType container;
  Iterator<typename BaseContainerType::iterator> provider;
};

template <class Provider>
class Get final
{
public:
  using value_type = typename Provider::value_type;

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

  std::shared_ptr<value_type> GetValue() {
    return provider.GetValue();
  }

private:
  Provider provider;
  size_t current = 0;
  size_t amount;
};

template <class Provider, class Transform>
class Map final
{
public:
  using value_type = std::invoke_result_t<Transform, typename Provider::value_type>;

  Map(Provider&& provider, Transform&& transform) :
  provider(std::move(provider)),
  transform(std::forward<Transform>(transform))
  {}

  bool Advance() {
    return provider.Advance();
  }

  std::shared_ptr<value_type> GetValue() {
    return std::make_shared<value_type>(
      transform(std::move(*provider.GetValue())));
  }

private:
  Provider provider;
  Transform transform;
};

template <class Provider, class Predicate>
class Filter final
{
public:
  using value_type = typename Provider::value_type;

  Filter(Provider&& provider, Predicate&& predicate) :
  provider(std::move(provider)),
  predicate(std::forward<Predicate>(predicate))
  {}

  bool Advance() {
    while (provider.Advance()) {
      current = provider.GetValue();
      if (predicate(*current))
        return true;
    }
    current.reset();
    return false;
  }

  std::shared_ptr<value_type> GetValue() {
    return current;
  }

private:
  Provider provider;
  Predicate predicate;
  std::shared_ptr<value_type> current;
};

template <class Provider>
class Group final
{
public:
  using value_type = std::vector<typename Provider::value_type>;

  Group(Provider&& provider, size_t size) :
  provider(std::move(provider)),
  size(size)
  {}

  bool Advance() {
    if (streamEnded) {
      current.reset();
      return false;
    }
    current = std::make_shared<value_type>();
    for (size_t i = 0; i < size; ++i) {
      if (provider.Advance()) {
        current->emplace_back(std::move(*provider.GetValue()));
      } else {
        streamEnded = true;
        break;
      }
    }
    return true;
  }

  std::shared_ptr<value_type> GetValue() {
    return current;
  }

private:
  Provider provider;
  size_t size;
  std::shared_ptr<value_type> current;
  bool streamEnded = false;
};

namespace traits {

template <class Provider>
struct is_finite {};

template <class IteratorType>
struct is_finite<Iterator<IteratorType>> : std::true_type {};

template <class GeneratorType>
struct is_finite<Generator<GeneratorType>> : std::false_type {};

template <class ContainerType>
struct is_finite<Container<ContainerType>> : std::true_type {};

template <class Provider>
struct is_finite<Get<Provider>> : std::true_type {};

template <class Provider, class Transform>
struct is_finite<Map<Provider, Transform>> : is_finite<Provider> {};

template <class Provider, class Predicate>
struct is_finite<Filter<Provider, Predicate>> : is_finite<Provider> {};

template <class Provider>
struct is_finite<Group<Provider>> : is_finite<Provider> {};

template <class Provider>
constexpr bool is_finite_v = is_finite<Provider>::value;

template <class T>
struct is_provider : std::false_type {};

template <class IteratorType>
struct is_provider<Iterator<IteratorType>> : std::true_type {};

template <class GeneratorType>
struct is_provider<Generator<GeneratorType>> : std::true_type {};

template <class ContainerType>
struct is_provider<Container<ContainerType>> : std::true_type {};

template <class Provider>
struct is_provider<Get<Provider>> : is_provider<Provider> {};

template <class Provider, class Transform>
struct is_provider<Map<Provider, Transform>> : is_provider<Provider> {};

template <class Provider, class Predicate>
struct is_provider<Filter<Provider, Predicate>> : is_provider<Provider> {};

template <class Provider>
struct is_provider<Group<Provider>> : is_provider<Provider> {};

template <class Provider>
constexpr bool is_provider_v = is_provider<Provider>::value;

} // namespace traits
} // namespace providers
} // namespace stream

#endif