namespace stream
{
  namespace providers
  {
    class EmptyStreamException;

    template <class Derived>
    class ProviderInterface;

    namespace core
    {
      template <class IteratorType>
      class Iterator;

      template <class GeneratorType>
      class Generator;

      template <class ContainerType>
      class Container;

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
      } // namespace traits
    } // namespace core

    namespace composite
    {
      template <class Provider, class Derived>
      class CompositeProvider;

      template <class Provider>
      class Get;

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

        template <class Provider>
        struct is_finite<Get<Provider>>;

        template <class Provider>
        struct is_finite<Skip<Provider>>;

        template <class Provider, class Transform>;
        struct is_finite<Map<Provider, Transform>>;

        template <class Provider, class Predicate>;
        struct is_finite<Filter<Provider, Predicate>>;

        template <class Provider>
        struct is_finite<Group<Provider>>;
      } // namespace traits
    } // namespace composite

    namespace traits
    {
      template <class Provider>
      struct is_finite;
    } // namespace traits
  } // namespace providers

  // wrappers for operators and terminators
  // easier usage with streams
  template <class> class Operator;
  template <class> class Terminator;

  // Core types
  // THE stream class
  template <class Provider>
  class Stream;

  // Wrapper for operators
  // Requirements:
  // F :: Stream<A> -> Stream<B<A>>
  template <class F>
  class Operator;

  // Wrapper for terminators
  // Requirements:
  // F :: Stream<A> -> B
  template <class F>
  class Terminator;

  template <class F, class G>
  class Compose;

  namespace operators
  {
    // Define callables with types
    // F<B> :: Stream<A> -> Stream<B<A>>

    class Get;
    
    class Skip;

    template <class Transform>
    class Map;

    template <class Predicate>
    class Filter;

    class Group;
  } // namespace operators

  namespace terminators
  {
    // Define callables with types
    // F :: Stream<A> -> B

    template <class IdentityFn, class Accumulator>
    class Reduce;

    class PrintTo;
    
    class ToVector;
    
    class Nth;

    namespace traits
    {
      template <class Terminator>
      struct supports_infinite;

      template <class Terminator, class Operator>
      struct supports_infinite<Compose<Terminator, Operator>>;

      template <class IdentityFn, class Accumulator>
      struct supports_infinite<Reduce<IdentityFn, Accumulator>>;

      template <> struct supports_infinite<PrintTo>;
      template <> struct supports_infinite<ToVector>;
      template <> struct supports_infinite<Nth>;
    } // namespace traits
  } // namespace terminators

  // Stream interface

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
} // namespace stream