#include <memory>
#include <utility>
#include <type_traits>

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
} // namespace

namespace stream 
{

template <class> class Operator;
template <class> class Terminator;

template <class T>
class Stream
{
public:
  template <class Iter>
  Stream(Iter begin, Iter end) :
    source(std::make_unique<providers::Iterator<T, Iter>>(begin, end)) {}

  template <class Container,
    class = std::enable_if_t<is_container_v<Container>>>
  Stream(const Container& cont) :
    Stream(cont.begin(), cont.end()) {}

  template <class Container,
    class = std::enable_if_t<is_container_v<Container>>>
  Stream(Container&& cont) :
    source(std::make_unique<providers::ContainerOwner<Container>>(std::move(cont))) {}
  
  Stream(std::initializer_list<T> init) :
    source(std::make_unique<providers::ContainerOwner<
      std::initializer_list<T>>>(std::move(init))) {}

  template <class Generator, 
    class = std::enable_if_t<std::is_invocable_v<Generator()>>, 
    class = int>
  Stream(Generator&& generator) :
    source(std::make_unique<providers::Generate<Generator>>(
      std::forward<Generator>(generator))) {}

  /*
  template <class ...Args>
  Stream(Args... args)
    source(std::make_unique<providers::Pack>(args...)) {};
  */

  template <class F>
  auto operator|(Operator<F>&& op) {
    return op.apply(std::move(*this));
  }

  template <class F>
  auto operator|(Operator<F>& op) {
    return op.apply(std::move(*this));
  }

  template <class F>
  auto operator|(Terminator<F>&& term) {
    return term.apply(std::move(*this));
  }

  template <class F>
  auto operator|(Terminator<F>& term) {
    return term.apply(std::move(*this));
  }

  StreamProviderPtr<T>& GetSource() { return source; }

private:
  StreamProviderPtr<T> source;

  template<class> friend class Operator;
  template<class> friend class Terminator;
};

template <class Iter> Stream(Iter, Iter) ->
  Stream<typename Iter::value_type>;
template <class Container, class = std::enable_if_t<is_container_v<Container>>> Stream(const Container&) ->
  Stream<typename Container::value_type>;
template <class Container, class = std::enable_if_t<is_container_v<Container>>> Stream(Container&&) ->
  Stream<typename Container::value_type>;
template <class Generator, class = std::enable_if_t<std::is_invocable_v<Generator()>>, class = int> Stream(Generator&&) ->
  Stream<std::result_of_t<Generator()>>;

} // namespace stream

#include "StreamOperations.h"
#include "StreamOperators.h"
