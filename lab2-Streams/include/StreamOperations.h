#ifndef INCLUDE_STREAMOPERATIONS_H
#define INCLUDE_STREAMOPERATIONS_H

#include "StreamProviders.h"
#include "StreamTerminators.h"
#include "StreamOperators.h"

namespace stream {

template <class> class Stream;

template <class F>
class Terminator
{
public:
  Terminator(F&& term) : 
    term(std::forward<F>(term))
  {}

  template <class Provider>
  auto Apply(Stream<Provider>&& stream) -> 
  std::invoke_result_t<F, Stream<Provider>&&> {
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
  Operator(F&& op) :
    op(std::forward<F>(op))
  {}

  template <class Provider>
  auto Apply(Stream<Provider>&& stream) {
    return op(std::move(stream));
  }

  template<class> friend class Operator;

private:
  F op;
};

} // namespace stream

#endif
