#ifndef INCLUDE_STREAMOPERATIONS_H
#define INCLUDE_STREAMOPERATIONS_H

#include <optional>

namespace stream {

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

auto get(size_t n) {
  return Operator(operators::Get(n));
}

template <class Transform>
auto map(Transform&& transform) {
  return Operator(operators::Map(std::forward<Transform>(transform)));
}

template <class Predicate>
auto filter(Predicate&& predicate) {
  return Operator(operators::Filter(std::forward<Predicate>(predicate)));
}

auto skip(size_t amount) {
  return Operator(operators::Skip(amount));
}

auto group(size_t size) {
  return Operator(operators::Group(size));
}


template <class Accumulator>
auto reduce(Accumulator&& accum) {
  return Terminator(terminators::Reduce(
    [](auto x) { return x; },
    std::forward<Accumulator>(accum)
  ));
}

template <class IdentityFn, class Accumulator>
auto reduce(IdentityFn&& identityFn, Accumulator&& accum) {
  return Terminator(terminators::Reduce(
    std::forward<IdentityFn>(identityFn),
    std::forward<Accumulator>(accum)
  ));
}

auto sum() {
  return Terminator(terminators::Reduce(
    [](auto x) { return x; },
    [](auto x, auto y) { return x + y; }
  ));
}

auto print_to(std::ostream& os, const char* delimiter = " ") {
  return Terminator(terminators::PrintTo(
    os, delimiter
  ));
}

auto to_vector() {
  return Terminator(terminators::ToVector());
}

auto nth(size_t index) {
  return Terminator(terminators::Nth(index));
}

} // namespace stream

#endif