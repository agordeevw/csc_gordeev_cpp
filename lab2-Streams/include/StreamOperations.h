#ifndef LAB2_STREAMS_INCLUDE_STREAMOPERATIONS_H_
#define LAB2_STREAMS_INCLUDE_STREAMOPERATIONS_H_

#include <utility>

#include "StreamProviders.h"
#include "StreamTerminators.h"
#include "StreamOperators.h"

namespace stream {

template <class> class Stream;

template <class F, class G>
class Compose
{
public:
  Compose(F&& f, G&& g):
    f(std::forward<F>(f)),
    g(std::forward<G>(g))
  {}

  template <class Provider>
  auto operator()(Stream<Provider>&& stream) {
    return f(g(std::move(stream)));
  }

private:
  F f;
  G g;
};

template <class F>
class Terminator
{
public:
  explicit Terminator(F&& term) :
    term(std::forward<F>(term))
  {}

  template <class Provider>
  auto Apply(Stream<Provider>&& stream)
    -> std::invoke_result_t<F, Stream<Provider>&&> {
    static_assert(
      providers::traits::is_finite_v<Provider>
        || terminators::traits::supports_infinite_v<F>,
      "Terminator doesn\'t support infinite streams");
    return term(std::move(stream));
  }

  template<class> friend class Operator;

private:
  F term;
};

template <class F>
class Operator
{
public:
  explicit Operator(F&& op) :
    op(std::forward<F>(op))
  {}

  template <class Provider>
  auto Apply(Stream<Provider>&& stream) {
    return op(std::move(stream));
  }

  template <class G>
  auto operator|(Operator<G>&& other) {
    return Operator<Compose<G, F>>(
      Compose(std::move(other.op), std::move(op)));
  }

  template <class G>
  auto operator|(Terminator<G>&& other) {
    return Terminator<Compose<G, F>>(
      Compose(std::move(other.term), std::move(op)));
}

  template<class> friend class Operator;

private:
  F op;
};

}  // namespace stream

#endif  // LAB2_STREAMS_INCLUDE_STREAMOPERATIONS_H_
