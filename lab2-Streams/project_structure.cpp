namespace stream
{
  class EmptyStreamException;

  namespace providers
  {
    template <class IteratorType>
    class Iterator;

    template <class GeneratorType>
    class Generator;

    template <class ContainerType>
    class Container;

    template <class Provider>
    class Get;

    template <class Provider>
    class Skip;

    template <class Provider, class Transform>
    class Map;

    template <class Provider, class Predicate>
    class Filter;

    template <class Provider>
    class Group;

    namespace traits
    {
      template <class Provider>
      struct is_finite;

      template <class IteratorType>
      struct is_finite<Iterator<IteratorType>>;

      template <class GeneratorType>
      struct is_finite<Generator<GeneratorType>>;

      template <class Container>
      struct is_finite<Container<ContainerType>>;

      template <class Provider>
      struct is_finite<Get<Provider>>;

      template <class Provider>
      struct is_finite<Skip<Provider>>;

      template <class Provider, class Transform>
      struct is_finite<Map<Provider, Transform>>;

      template <class Provider, class Predicate>
      struct is_finite<Filter<Provider, Predicate>>;

      template <class Provider>
      struct is_finite<Group<Provider>>;

      template <class T>
      struct is_provider;

      template <class IteratorType>
      struct is_provider;

      template <class GeneratorType>
      struct is_provider;

      template <class ContainerType>
      struct is_provider;

      template <class Provider>
      struct is_provider;

      template <class Provider>
      struct is_provider;

      template <class Provider, class Transform>
      struct is_provider;

      template <class Provider, class Predicate>
      struct is_provider;

      template <class Provider>
      struct is_provider;

    } // namespace traits
  } // namespace providers

  template <class F, class G>
  class Compose;

  template <class F>
  class Terminator;

  template <class F>
  class Operator;

  namespace terminators {
    template <class IdentityFn, class Accumulator>
    class Reduce;

    class PrintTo;

    class ToVector;

    class Nth;

    namespace traits
    {
      template <class>
      struct supports_infinite {};

      template <class I, class A>
      struct supports_infinite<Reduce<I, A>>;

      template <>
      struct supports_infinite<PrintTo>;

      template <>
      struct supports_infinite<ToVector>;

      template <>
      struct supports_infinite<Nth>;

      template <class Term, class Op>
      struct supports_infinite<Compose<Term, Op>>;

    } // namespace traits
  } // namespace terminators

  namespace operators {
    class Get;

    class Skip;

    template <class Transform>
    class Map;

    template <class Predicate>
    class Filter;

    class Group;

  } // namespace operators

  auto get(size_t n);

  template <class Transform>
  auto map(Transform&& transform);

  template <class Accumulator>
  auto reduce(Accumulator&& accum);

  template <class IdentityFn, class Accumulator>
  auto reduce(IdentityFn&& identityFn, Accumulator&& accum);

  template <class Predicate>
  auto filter(Predicate&& predicate);

  auto skip(std::size_t amount);

  auto group(std::size_t n);

  auto sum();

  auto print_to(std::ostream& os, const char* delimiter = " ");

  auto to_vector();

  auto nth(std::size_t index);

  template <class Provider>
  class Stream;

} // namespace stream
