#ifndef INCLUDE_STREAMPROVIDERS_H
#define INCLUDE_STREAMPROVIDERS_H

#include <memory>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace stream {
namespace providers {

class EmptyStreamException : public std::logic_error
{
public:
  EmptyStreamException() : 
  std::logic_error("ERROR: Empty stream") {};
};

template <class Derived>
class ProviderInterface
{
public:
  using value_type = typename Derived::value_type;

  bool Advance() { return AdvanceImpl(); }
  std::shared_ptr<value_type> Get() { return GetImpl(); }
};

namespace core {

template <class IteratorType>
class Iterator : public ProviderInterface<Iterator<IteratorType>>
{
public:
  using value_type = typename IteratorType::value_type;

  Iterator(IteratorType begin, IteratorType end) :
  current(begin), end(end)
  {}

private:
  bool AdvanceImpl() {
    if (first) {
      first = false;
      return current != end;
    }
    ++current;
    return current != end;
  }

  std::shared_ptr<value_type> GetImpl() {
    return std::make_shared<value_type>(*current);
  }

  bool first = true;
  Iter current;
  Iter end;
};

template <class GeneratorType>
class Generator : public ProviderInterface<Generator<GeneratorType>>
{
public:
  using value_type = std::invoke_result_t<GeneratorType>;

  Generator(GeneratorType&& generator) :
  generator(std::forward<GeneratorType>(generator))
  {}

private:
  bool AdvanceImpl() {
    current = std::make_shared<T>(generator());
    return true;
  }

  std::shared_ptr<value_type> GetImpl() {
    return current;
  }

  GeneratorType generator;
  std::shared_ptr<value_type> current;
};

template <class ContainerType>
class Container : public ProviderInterface<Container<ContainerType>>
{
public:
  using value_type = typename ContainerType::value_type;

  Container(ContainerType&& container) :
  container(std::move(container)),
  provider(this->container.begin(), this->container.end())
  {}

private:
  bool AdvanceImpl() { return provider.Advance(); }

  std::shared_ptr<value_type> GetImpl() { return provider.Get(); }

  ContainerType container;
  Iterator<typename ContainerType::iterator> provider;
};

namespace traits {

template <class Provider>
struct is_finite
{
  static constexpr std::optional<bool> optvalue = std::nullopt;
};

template <class IteratorType>
struct is_finite<Iterator<IteratorType>>
{
  static constexpr std::optional<bool> optvalue = true;
};

template <class GeneratorType>
struct is_finite<Generator<GeneratorType>>
{
  static constexpr std::optional<bool> optvalue = false;
};

template <class ContainerType>
struct is_finite<Container<ContainerType>>
{
  static constexpr std::optional<bool> optvalue = true;
}

} // namespace traits
} // namespace core

namespace composite {

template <class Provider, class Derived>
class CompositeProvider : 
public ProviderInterface<CompositeProvider<Provider, Derived>>
{
public:
  using value_type = typename Derived::value_type;

  CompositeProvider(Provider&& provider) :
  provider(std::move(provider)) // or forward?
  {}

protected:
  Provider provider;
};

template <class Provider>
class Get final : public CompositeProvider<Provider, Get<Provider>>
{
public:
  using value_type = typename Provider::value_type;

  Get(Provider&& provider, size_t amount) :
  CompositeProvider(std::move(provider)), amount(amount)
  {}

private:
  bool AdvanceImpl() {
    if (current < amount) {
      ++current;
      return provider.Advance();
    }
    return false;
  }

  std::shared_ptr<value_type> AdvanceImpl() {
    return provider.Get();
  }

  size_t amount;
};

template <class Provider, class Transform>
class Map final : public CompositeProvider<Provider, Map<Provider, Transform>>
{
public:
  using value_type = std::invoke_result_t<Transform, typename Provider::value_type>>;

  Map(Provider&& provider, Transform&& transform) :
  CompositeProvider(std::move(provider)),
  transform(std::forward<Transform>(transform))
  {}

private:
  bool AdvanceImpl() {
    return provider.Advance();
  }

  std::shared_ptr<value_type> GetImpl() {
    return std::make_shared<value_type>(transform(*provider.Get()));
  }

  Transform transform;
};

template <class Provider, class Predicate>
class Filter final : public CompositeProvider<Provider, Filter<Provider, Predicate>>
{
public:
  using value_type = typename Provider::value_type;

  Filter(Provider&& provider, Predicate&& predicate) :
  CompositeProvider(std::move(provider)),
  predicate(std::forward<Predicate>(predicate))
  {}

private:
  bool AdvanceImpl() {
    while (provider.Advance()) {
      current = provider.Get();
      if (predicate(*current))
        return true;
    }
    current.reset();
    return false;
  }

  std::shared_ptr<value_type> GetImpl() {
    return current;
  }

  Predicate predicate;
  std::shared_ptr<value_type> current;
};

template <class Provider>
class Group final : public CompositeProvider<Provider, Group<Provider>>
{
public:
  using value_type = std::vector<typename Provider::value_type>;

  Group(Provider&& provider, size_t size) :
  CompositeProvider(std::move(provider)), size(size)
  {}

private:
  bool AdvanceImpl() {
    if (streamEnded) {
      current.reset();
      return false;
    }
    current = std::make_shared<value_type>();
    for (size_t i = 0; i < size; ++i) {
      if (provider.Advance()) {
        current->emplace_back(std::move(*provider.Get()));
      } else {
        streamEnded = true;
        break;
      }
    }
    return true;
  }

  std::shared_ptr<value_type> GetImpl() {
    return current;
  }

  bool streamEnded = false;
  size_t size;
  std::shared_ptr<value_type> current;
};

namespace traits {

template <class Provider>
struct is_finite
{
  static constexpr std::optional<bool> optvalue = std::nullopt;
};

template <class Provider>
struct is_finite<Get<Provider>>
{
  static constexpr std::optional<bool> optvalue = true;
};

template <class Provider, class Transform>
struct is_finite<Map<Provider, Transform>>
{
  static constexpr std::optional<bool> optvalue =
    is_finite<Provider>::optvalue;
};

template <class Provider, class Predicate>
struct is_finite<Filter<Provider, Predicate>>
{
  static constexpr std::optional<bool> optvalue = 
    is_finite<Provider>::optvalue;
};

template <class Provider>
struct is_finite<Group<Provider>>
{
  static constexpr std::optional<bool> optvalue =
    is_finite<Provider>::optvalue;
};

} // namespace traits
} // namespace composite

namespace traits {

template <class Provider>
struct is_finite
{
private:
  static constexpr bool DetermineValue() {
    constexpr auto coreValue = 
      core::traits::is_finite<Provider>::optvalue;
    constexpr auto compValue = 
      composite::traits::is_finite<Provider>::optvalue;
    static_assert(coreValue.has_value() || compValue.has_value(),
        "Provider type doesn\'t match with known types, 
         consider implementing corresponding traits");
    if constexpr(coreValue.has_value())
      return coreValue.value();
    if constexpr(compValue.has_value())
      return compValue.value();
  }

public:
  static constexpr bool value = DetermineValue();
};

template <class Provider>
constexpr bool is_finite_v = is_finite<Provider>::value;
 
} // namespace traits

} // namespace providers
} // namespace stream
