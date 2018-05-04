#ifndef INCLUDE_STREAM_H
#define INCLUDE_STREAM_H

#include <memory>
#include <utility>
#include <iterator>
#include <type_traits>
#include <list>

#include "StreamProviders.h"

namespace {
  template <class T, class _ = void>
  struct is_container: std::false_type {};

  template <class... Ts>
  struct is_container_helper {};

  template <class T>
  struct is_container<
    T,
    std::conditional_t<
      false,
      is_container_helper<
        decltype(std::declval<T>().begin()),
        decltype(std::declval<T>().end())
        >,
      void
      >
    > : std::true_type {};

  template <class T>
  constexpr bool is_container_v = is_container<T>::value;

  template <class T, class ...Args>
  void unpack(std::list<T>& list, Args ... args) {
    (list.emplace_back(args), ...); 
  }

  template <class T, class ...Args>
  std::list<T> make_list_from(T arg, Args... args) {
    std::list<T> ret;
    ret.emplace_back(arg);
    unpack(ret, args...);
    return ret;
  }

  template <class T, class _ = void>
  struct is_iterator: std::false_type {};

  template <class... Ts>
  struct is_iterator_helper {};

  template <class T>
  struct is_iterator<
    T,
    std::conditional_t<
    false,
    is_iterator_helper<
      decltype(std::declval<T>().operator*()),
      decltype(std::declval<T>().operator++())
      >,
      void
      >
    > : std::true_type {};

  template <class T>
  constexpr bool is_iterator_v = is_iterator<T>::value;
} // namespace

namespace stream {
namespace util {

template <class T>
struct is_container {
  constexpr bool value = !std::is_same_v
    <
    std::iterator_traits<typename T::iterator>::value_type,
    void
    >;
  // check for begin(), end()?
}

template <class T>
constexpr bool is_container_v = is_container<T>::value;

template <class T, class ... Args>
void unpack(std::list<T>& list, Args ... args) {
  (list.emplace_back(args), ...);
}

template <class T, class ... Args>
std::list<T> CreateListFrom(T arg, Args ... args) {
  std::list<T> ret;
  ret.emplace_back(arg);
  unpack(ret, args...);
  return ret;
}

} // namespace util

template <class> class Operator;
template <class> class Terminator;

template <class Provider>
class Stream
{
public:
  using value_type = typename Provider::value_type;

  template <class Iterator>
  Stream(std::enable_if_t
    <
    !std::is_same_v<std::iterator_traits<Iterator>::value_type, void>,
    Iterator
    > begin, Iterator end) : 
  provider(begin, end) {};

  template <class Generator>
  Stream(std::enable_if_t
    <
    std::is_invocable_v<Generator>
    && !std::is_same_v<std::invoke_result_t<Generator>, void>,
    Generator&&
    > generator) : 
  provider(std::forward<Generator>(generator)) {}

  template <class Container>
  Stream(std::enable_if_t
    <
    is_container_v<Container>, 
    const Container&
    > container) :
  Stream(container.begin(), container.end()) {}

  template <class Container>
  Stream(std::enable_if_t
    <
    is_container_v<Container>,
    Container&&
    > container) :
  provider(std::move(container)) {}

  template <class T>
  Stream(std::initializer_list<T> init) :
  provider(std::move(init)) {}

  template <class T, class ... Args>
  Stream(T arg, Args ... args) :
  provider(CreateListFrom(arg, args...)) {}

  Stream(Provider&& provider) : 
  provider(std::forward<Provider>(provider)) {}

  Stream() = delete;
  Stream(const Stream<Provider>&) = delete;
  Stream& operator=(const Stream<Provider>&) = delete;
  Stream& operator=(Stream<Provider>&&) = delete;
  Stream(Stream<Provider>&& stream) : provider(std::move(stream.provider)) {}

  template <class F>
  auto operator|(Operator<F>&& op) {
    return op.Apply(std::move(*this));
  }

  template <class F>
  auto operator|(Operator<F>& op) {
    return op.Apply(std::move(*this));
  }

  template <class F>
  auto operator|(Terminator<F>&& term) -> std::result_of_t<F(Stream&&)> { 
    return term.Apply(std::move(*this)); 
  }

  template <class F>
  auto operator|(Terminator<F>& term) -> std::result_of_t<F(Stream&&)> {
    return term.Apply(std::move(*this));
  }

  auto& GetProvider() { return provider; }

private:
  Provider provider;
};

template <class Iterator>
Stream(Iterator begin, Iterator end) -> 
  Stream<providers::core::Iterator<Iterator>>;

template <class Generator>
Stream(Generator&& generator) ->
  Stream<providers::core::Generator<Generator>>;

} // namespace stream

#include "StreamOperations.h"
#include "StreamOperators.h"
#include "StreamTerminators.h"

#endif