#ifndef INCLUDE_STREAMPROVIDERS_H
#define INCLUDE_STREAMPROVIDERS_H

#include <memory>
#include <type_traits>
#include <list>
#include <stdexcept>
#include <optional>

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
class Container : public Iterator<typename ContainerType::iterator>
{
public:
  using value_type = typename ContainerType::value_type;

  Container(ContainerType&& container) :
  container(std::move(container))
  {
    current = this->container.begin()
    end = this->container.end();
  }

private:
  ContainerType container;
};

namespace traits {

template <class Provider>
struct is_finite
{
  constexpr std::optional<bool> optvalue = std::nullopt;
};

template <class IteratorType>
struct is_finite<Iterator<IteratorType>>
{
  constexpr std::optional<bool> optvalue = true;
};

template <class GeneratorType>
struct is_finite<Generator<GeneratorType>>
{
  constexpr std::optional<bool> optvalue = false;
};

template <class ContainerType>
struct is_finite<Container<ContainerType>>
{
  constexpr std::optional<bool> optvalue = true;
}

} // namespace traits
} // namespace core

namespace composite {

namespace traits {

} // namespace traits
} // namespace composite

} // namespace providers
} // namespace stream