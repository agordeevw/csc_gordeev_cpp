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
  std::logic_error("Empty stream") {};
};

template <class Derived>
class ProviderInterface
{
public:
  bool Advance() {
    return Impl().Advance();
  }
  auto GetValue() {
    return Impl().Get();
  }

  ProviderInterface& Impl() {
    return static_cast<Derived&>(*this);
  }
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
class Generator : public ProviderInterface<Generator<GeneratorType>>
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
class Container : public ProviderInterface<Container<ContainerType>>
{
public:
  using value_type = typename ContainerType::value_type;

  Container(ContainerType&& container) :
  container(std::move(container)),
  provider(this->container.begin(), this->container.end())
  {}

  bool Advance() {
    return provider.Advance();
  }

  std::shared_ptr<value_type> GetValue() {
    return provider.GetValue();
  }

private:
  ContainerType container;
  Iterator<typename ContainerType::iterator> provider;
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
constexpr bool is_finite_v = is_finite<Provider>::value;

template <class T>
struct is_provider : std::bool_constant<
  std::is_convertible_v<T, ProviderInterface<T>>
> {};

} // namespace traits
} // namespace core

namespace composite {

template <class Provider, class Derived>
class CompositeProvider : 
public ProviderInterface<CompositeProvider<Provider, Derived>>
{
public:
  CompositeProvider(Provider&& provider) :
  provider(std::move(provider)) // or forward?
  {}

  auto Advance() {
    return Impl().Advance();
  }

  auto GetValue() {
    return Impl().GetValue();
  }

  CompositeProvider& Impl() {
    return static_cast<Derived&>(*this);
  }

protected:
  Provider provider;
};

template <class Provider>
class Get final : public CompositeProvider<Provider, Get<Provider>>
{
public:
  using value_type = typename Provider::value_type;

  Get(Provider&& provider, size_t amount) :
  CompositeProvider<Provider, Get<Provider>>(std::move(provider)), 
  amount(amount)
  {}

  bool Advance() {
    if (current < amount) {
      ++current;
      return this->provider.Advance();
    }
    return false;
  }

  std::shared_ptr<value_type> GetValue() {
    return this->provider.GetValue();
  }

private:
  size_t current = 0;
  size_t amount;
};

template <class Provider, class Transform>
class Map final : public CompositeProvider<Provider, Map<Provider, Transform>>
{
public:
  using value_type = std::invoke_result_t<Transform, typename Provider::value_type>;

  Map(Provider&& provider, Transform&& transform) :
  CompositeProvider<Provider, Map<Provider, Transform>>(std::move(provider)),
  transform(std::forward<Transform>(transform))
  {}

  bool Advance() {
    return this->provider.Advance();
  }

  std::shared_ptr<value_type> GetValue() {
    return std::make_shared<value_type>(
      transform(std::move(*this->provider.Get())));
  }

private:
  Transform transform;
};

template <class Provider, class Predicate>
class Filter final : public CompositeProvider<Provider, Filter<Provider, Predicate>>
{
public:
  using value_type = typename Provider::value_type;

  Filter(Provider&& provider, Predicate&& predicate) :
  CompositeProvider<Provider, Filter<Provider, Predicate>>(std::move(provider)),
  predicate(std::forward<Predicate>(predicate))
  {}

  bool Advance() {
    while (this->provider.Advance()) {
      current = this->provider.GetValue();
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
  Predicate predicate;
  std::shared_ptr<value_type> current;
};

template <class Provider>
class Group final : public CompositeProvider<Provider, Group<Provider>>
{
public:
  using value_type = std::vector<typename Provider::value_type>;

  Group(Provider&& provider, size_t size) :
  CompositeProvider<Provider, Group<Provider>>(std::move(provider)), 
  size(size)
  {}

  bool Advance() {
    if (streamEnded) {
      current.reset();
      return false;
    }
    current = std::make_shared<value_type>();
    for (size_t i = 0; i < size; ++i) {
      if (this->provider.Advance()) {
        current->emplace_back(std::move(*this->provider.GetValue()));
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
  bool streamEnded = false;
  size_t size;
  std::shared_ptr<value_type> current;
};

namespace traits {

template <class Provider>
struct is_finite {};

template <class Provider>
struct is_finite<Get<Provider>> : std::true_type {};

template <class Provider, class Transform>
struct is_finite<Map<Provider, Transform>> :
is_finite<Provider>, core::traits::is_finite<Provider> {};

template <class Provider, class Predicate>
struct is_finite<Filter<Provider, Predicate>> :
is_finite<Provider>, core::traits::is_finite<Provider> {};

template <class Provider>
struct is_finite<Group<Provider>> :
is_finite<Provider>, core::traits::is_finite<Provider> {};

template <class T>
struct is_provider : std::false_type {};

template <template <class...> class A, class P, class ... Ts>
struct is_provider<A<P, Ts...>> : std::bool_constant<
  std::is_convertible_v<A<P, Ts...>, CompositeProvider<P, A<P, Ts...>>>
  && (is_provider<P>::value || core::traits::is_provider<P>::value)
> {};

} // namespace traits
} // namespace composite

namespace traits {

template <class Provider>
struct is_finite : 
core::traits::is_finite<Provider>,
composite::traits::is_finite<Provider> {};

template <class Provider>
constexpr bool is_finite_v = is_finite<Provider>::value;

template <class T>
struct is_provider : std::bool_constant<
  core::traits::is_provider<T>::value
  || composite::traits::is_provider<T>::value
> {};

template <class T>
constexpr bool is_provider_v = is_provider<T>::value;

} // namespace traits

} // namespace providers
} // namespace stream

#endif