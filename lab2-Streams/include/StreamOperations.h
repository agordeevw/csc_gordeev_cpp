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

namespace operators {
  class Get;
  class Skip;
  template <class> class Map;
  template <class> class Filter;
  class Group;
} // namespace operators

namespace terminators {
  template <class, class> Reduce;
  class PrintTo;
  class ToVector;
  class Nth;

  namespace traits
  {
    template <class Term>
    struct supports_infinite_helper {
      static constexpr std::optional<bool> optvalue = std::nullopt;
    };

    template <class Term, class Op>
    struct supports_infinite_helper<Compose<Term, Op>> {
      static constexpr std::optional<bool> optvalue = 
        supports_infinite<Term>::value;
    };

    template <class I, class A>
    struct supports_infinite_helper<Reduce<I, A>> {
      static constexpr std::optional<bool> optvalue = false;
    };

    template <>
    struct supports_infinite_helper<PrintTo> {
      static constexpr std::optional<bool> optvalue = false;
    };

    template <>
    struct supports_infinite_helper<ToVector> {
      static constexpr std::optional<bool> optvalue = false;
    };

    template <>
    struct supports_infinite_helper<NthTerminator> {
      static constexpr std::optional<bool> optvalue = true;
    };

    template <class Term>
    struct supports_infinite {
    private:
      static constexpr bool DetermineValue() {
        constexpr auto optvalue = 
          supports_infinite_helper<Term>::optvalue;
        static_assert(optvalue.has_value(),
          "Terminator type doesn\' match with known types,
          consider implementing corresponding traits");
        return optvalue.value();
      }
    public:
      static constexpr bool value = DetermineValue();
    };

    template <class Term>
    constexpr bool supports_infinite_v = supports_infinite<Term>::value;

  } // namespace traits
} // namespace terminators

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