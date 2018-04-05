#include <memory>
#include <utility>

#include "StreamProviders.h"

namespace stream 
{

template <class T> class Operator;
template <class T> class Terminator;

template <class T>
class Stream
{
public:
  template <class Iter>
  Stream(Iter begin, Iter end) :
    source(std::make_unique<providers::Iterator<T, Iter>>(begin, end)) {}

  /*template <class Container>
  Stream(const Container& cont)
    Stream(cont.begin(), cont.end()) {}

  template <class Container>
  Stream(Container&& cont) :
    source(std::make_unique<providers::Container>(std::move(cont))) {}

  Stream(std::initializer_list<T> init) :
    source(std::make_unique<providers::Container>(std::move(init))) {}

  template <class Generator>
  Stream(Generator&& generator) :
    source(std::make_unique<providers::Generate>(
      std::forward<Generator>(generator)) {}*/

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

  std::unique_ptr<providers::StreamProvider<T>>& GetSource() { return source; }

private:
  std::unique_ptr<providers::StreamProvider<T>> source;

  template<class> friend class Operator;
  template<class> friend class Terminator;
};

template <class Iter> Stream(Iter, Iter) ->
  Stream<typename Iter::value_type>;

} // namespace stream

#include "StreamOperations.h"
#include "StreamOperators.h"
