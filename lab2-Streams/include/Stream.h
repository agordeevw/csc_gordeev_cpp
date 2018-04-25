#ifndef INCLUDE_STREAM_H
#define INCLUDE_STREAM_H

#include <memory>
#include <utility>
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

namespace stream 
{

template <class> class Operator;
template <class> class Terminator;

template <class T>
class Stream
{
public:
  using value_type = T;

  template <class Iter,
    class = std::enable_if_t<is_iterator_v<Iter>>>
  Stream(Iter begin, Iter end) :
    source(std::make_unique<providers::Iterator<T, Iter>>(begin, end)) {}

  template <class Container,
    class = std::enable_if_t<is_container_v<Container>>>
  Stream(const Container& cont) :
    Stream(cont.begin(), cont.end()) {}

  template <class Container,
    class = std::enable_if_t<is_container_v<Container>>>
  Stream(Container& cont) :
    Stream(cont.begin(), cont.end()) {}

  template <class Container,
    class = std::enable_if_t<is_container_v<Container>>>
  Stream(Container&& cont) :
    source(std::make_unique<providers::ContainerOwner<Container>>(std::move(cont))) {}
  
  Stream(std::initializer_list<T> init) :
    source(std::make_unique<providers::ContainerOwner<
      std::initializer_list<T>>>(std::move(init))) {}

  Stream(std::unique_ptr<providers::StreamProvider<T>>&& source) : 
    source(std::move(source)) {}

  template <class Generator, 
    class = std::enable_if_t<std::is_member_function_pointer_v<decltype(&(Generator::operator()))>>, 
    class = int>
  Stream(Generator&& generator) :
    source(std::make_unique<providers::Generate<Generator>>(
      std::forward<Generator>(generator))) {}

  template <class ...Args>
  Stream(T arg, Args... args) :
    source(std::make_unique<providers::ContainerOwner<std::list<T>>>(make_list_from(arg, args...))) {}

  template <class F>
  auto operator|(Operator<F>&& op) {
    return op.apply(std::move(*this));
  }

  template <class F>
  auto operator|(Operator<F>& op) {
    return op.apply(std::move(*this));
  }

  template <class F>
  auto operator|(Terminator<F>&& term) -> std::result_of_t<F(Stream<T>&&)> {
    return term.apply(std::move(*this));
  }

  template <class F>
  auto operator|(Terminator<F>& term) -> std::result_of_t<F(Stream<T>&&)> {
    return term.apply(std::move(*this));
  }

  StreamProviderPtr<T>& GetSource() { return source; }

private:
  StreamProviderPtr<T> source;

  template<class> friend class Operator;
  template<class> friend class Terminator;
};

template <class Iter, class = std::enable_if_t<is_iterator_v<Iter>>> Stream(Iter, Iter) ->
  Stream<typename Iter::value_type>;
template <class Container, class = std::enable_if_t<is_container_v<Container>>> Stream(Container&) ->
  Stream<typename Container::value_type>;
template <class Container, class = std::enable_if_t<is_container_v<Container>>> Stream(const Container&) ->
  Stream<typename Container::value_type>;
template <class Container, class = std::enable_if_t<is_container_v<Container>>> Stream(Container&&) ->
  Stream<typename Container::value_type>;
template <class Generator, class = std::enable_if_t<std::is_invocable_v<Generator()>>, class = int> Stream(Generator&&) ->
  Stream<std::result_of_t<Generator()>>;

} // namespace stream

#include "StreamOperations.h"
#include "StreamOperators.h"
#include "StreamTerminators.h"

#endif