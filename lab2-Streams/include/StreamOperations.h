#ifndef INCLUDE_STREAMOPERATIONS_H
#define INCLUDE_STREAMOPERATIONS_H

namespace stream
{
template <class F, class G>
class Compose
{
public:
  Compose(F&& f, G&&g) : f(std::forward<F>(f), std::forward<G>(g)) {}

  template <class T>
  auto operator()(Stream<T>&& stream) {
    return f(g(std::forward<Stream<T>>(stream)));
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

  template <class T>
  auto apply(Stream<T>&& stream) {
    return op(std::move(stream));
  }

  template <class G>
  auto operator|(Operator<G>&& rhs) {
    return Operator<Compose<G, F>>(Compose(std::move(rhs.op), std::move(op)));
  }

  template <class G>
  auto operator|(Terminator<G>&& rhs) {
    return Terminator<Compose<G, F>>(Compose(std::move(rhs.op), std::move(op)));
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

  template <class T>
  auto apply(Stream<T>&& stream) -> std::result_of_t<F(Stream<T>&&)> {
    return term(std::move(stream));
  }

  template<typename> friend class Operator;

private:
  F term;
};

} // namespace stream

#endif