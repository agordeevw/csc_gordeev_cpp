#ifndef LAB2_STREAMS_INCLUDE_STREAM_H_
#define LAB2_STREAMS_INCLUDE_STREAM_H_

#include <iterator>
#include <type_traits>
#include <utility>
#include <vector>

#include "StreamProviders.h"
#include "StreamOperations.h"
#include "StreamInterface.h"

namespace stream {
namespace util {

template <class T, class = void>
struct is_container :
  std::false_type {};

template <class T>
struct is_container<T, std::conditional_t<
  false,
  std::void_t<
    typename T::value_type,
    typename T::iterator,
    decltype(std::declval<T>().begin()),
    decltype(std::declval<T>().end())
    >,
  void
  >
> : std::true_type {};

template <class T>
constexpr bool is_container_v = is_container<T>::value;

template <class T, class ... Args>
void Unpack(std::vector<T>& vec, Args&& ... args) {
  (vec.emplace_back(std::forward<Args>(args)), ...);
}

template <class T, class ... Args>
std::vector<T> CreateContainerFrom(T&& arg, Args&& ... args) {
  std::vector<T> ret;
  ret.emplace_back(std::forward<T>(arg));
  Unpack(ret, std::forward<Args>(args)...);
  return ret;
}

}  // namespace util

template <class Provider>
class Stream
{
public:
  static_assert(
    providers::traits::is_provider_v<Provider>,
    "Stream provider type is not one of known provider types");

  Stream() = delete;
  Stream(const Stream&) = delete;
  Stream& operator=(const Stream&) = delete;

  Stream(Stream&& other) :
    provider(std::move(other.provider))
  {}

  Stream& operator=(Stream&& other) {
    if (this != &other)
      provider = std::move(other.provider);
    return *this;
  }

  template <class Iterator, class =
    std::enable_if_t<
      !std::is_same_v<
        typename std::iterator_traits<Iterator>::value_type,
        void
      >
    >
  >
  Stream(Iterator begin, Iterator end) :
    provider(begin, end)
  {}

  template <class Container, class =
    std::enable_if_t<
      util::is_container_v<Container>,
      void
    >
  >
  explicit Stream(const Container& container) :
    provider(container.begin(), container.end())
  {}

  template <class Container, class =
    std::enable_if_t<
      util::is_container_v<Container>,
      void
    >
  >
  explicit Stream(Container& container) :
    provider(container.cbegin(), container.cend())
  {}

  template <class Container, class =
    std::enable_if_t<
      util::is_container_v<std::remove_reference_t<Container>>,
      void
    >
  >
  explicit Stream(Container&& container) :
    provider(std::move(container))
  {}

  template <class T>
  explicit Stream(std::initializer_list<T> init) :
    provider(init)
  {}

  template <class Generator, class =
    std::enable_if_t<
      std::is_invocable_v<Generator>,
      Generator
    >,
    class = void
  >
  explicit Stream(Generator&& generator) :
    provider(std::forward<Generator>(generator))
  {}

  template <class OtherProvider, class =
    std::enable_if_t<
      providers::traits::is_provider_v<OtherProvider>,
      OtherProvider
    >,
    class = void,
    class = void
  >
  explicit Stream(OtherProvider&& provider) :
    provider(std::move(provider))
  {}

  template <class T, class ... Args>
  Stream(T&& arg, Args&& ... args) :
  Stream(util::CreateContainerFrom(
    std::forward<T>(arg), std::forward<Args>(args)...)) {}

  template <class F>
  auto operator|(Operator<F>&& op) {
    return op.Apply(std::move(*this));
  }

  template <class F>
  auto operator|(Operator<F>& op) {
    return op.Apply(std::move(*this));
  }

  template <class F>
  auto operator|(Terminator<F>&& term)
    -> std::invoke_result_t<F, Stream&&> {
    return term.Apply(std::move(*this));
  }

  template <class F>
  auto operator|(Terminator<F>& term)
    -> std::invoke_result_t<F, Stream&&> {
    return term.Apply(std::move(*this));
  }

  auto& GetProvider() { return provider; }

private:
  Provider provider;
};

template <class Iterator, class =
  std::enable_if_t<
    !std::is_same_v<
      typename std::iterator_traits<Iterator>::value_type,
      void
    >
  >
>
Stream(Iterator begin, Iterator end)
  -> Stream<providers::Iterator<Iterator>>;

template <class Container, class =
  std::enable_if_t<
    util::is_container_v<Container>,
    void
  >
>
Stream(const Container& container)
  -> Stream<providers::Iterator<typename Container::const_iterator>>;

template <class Container, class =
  std::enable_if_t<
    util::is_container_v<Container>,
    void
  >
>
Stream(Container& container)
  -> Stream<providers::Iterator<typename Container::const_iterator>>;

template <class Container, class =
  std::enable_if_t<
    util::is_container_v<std::remove_reference_t<Container>>,
    Container
  >
>
Stream(Container&&)
  -> Stream<providers::Container<Container>>;

template <class T>
Stream(std::initializer_list<T>)
  -> Stream<providers::Container<std::vector<T>>>;

template <class Generator, class =
  std::enable_if_t<
    std::is_invocable_v<Generator>,
    Generator
  >,
  class = void>
Stream(Generator&& generator)
  -> Stream<providers::Generator<Generator>>;

template <class OtherProvider, class =
  std::enable_if_t<
    providers::traits::is_provider_v<OtherProvider>,
    OtherProvider
  >,
  class = void,
  class = void
>
Stream(OtherProvider&& provider)
  -> Stream<OtherProvider>;

template <class T, class ... Args>
Stream(T&& arg, Args&& ... args)
  -> Stream<providers::Container<std::vector<T>>>;

}  // namespace stream

#endif  // LAB2_STREAMS_INCLUDE_STREAM_H_
