#ifndef INCLUDE_STREAMOPERATIONS_H
#define INCLUDE_STREAMOPERATIONS_H

namespace stream
{
template <class F, class G>
class Compose
{
public:
  Compose(F&& f, G&&g) : f(std::forward<F>(f), std::forward<G>(g)) {}

  template <class Provider>
  auto operator()(Stream<Provider>&& stream) {
    return f(g(std::move(stream)));
  }

private:
  F f;
  G g;
};

template <class F>
class Operator
{
public:
  Operator(F&& op) : op(std::forward<F>(op)) {}

  template <class Provider>
  auto Apply(Stream<Provider>&& stream) {
    return op(std::move(stream));
  }

  template <class G>
  auto operator|(Operator<G>&& other) {
    return Operator(Compose(std::move(other.op), std::move(op)));
  }

  template <class G>
  auto operator|(Terminator<G>&& other) {
    return Terminator(Compose(std::move(other.term), std::move(op)));
  }

  template<class> friend class Operator;

private:
  F op;
};

template <class F>
class Terminator
{
public:
  Terminator(F&& term) : term(std::forward<F>(term)) {}

  template <class Provider>
  auto Apply(Stream<Provider>&& stream) -> 
  std::invoke_result_t<F, Stream<T>&&> {
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

} // namespace stream

#endif