namespace stream
{
  class EmptyStreamException;

  /*
    Data providers for streams.
    Provide common interface:
      bool Advance(),
      std::shared_ptr<value_type> GetValue().
    Advance() tries to advance the provider to the next element.
    GetValue() returns current element.
  */
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

    /*
      Each trait _MUST_ be specialized for each provider.
        Otherwise compilation errors are ensued.
    */
    namespace traits
    {
      /*
        Useful to check if stream can be used with some terminators.
        For example, it's a bad idea to use infinite stream with sum terminator.
      */
      template <class Provider>
      struct is_finite;

      /*
        Used to prevent compilation in case 
          when Stream class is specialized 
          with anything except existing providers.
      */
      template <class T>
      struct is_provider;

    } // namespace traits
  } // namespace providers

  /*
    Allows to compose operators and terminators with each other
      to create composite operators or composite terminators.
  */
  template <class F, class G>
  class Compose;

  /*
    Wrapper for Callable of type
      (Stream<A>&&) -> B
  */
  template <class F>
  class Terminator;

  /*
    Wrapper for Callable of type
      (Stream<A>&&) -> Stream<B>
  */
  template <class F>
  class Operator;

  /*
    Each class must be a Callable of type
      (Stream<A>&&) -> B
  */
  namespace terminators {
    template <class IdentityFn, class Accumulator>
    class Reduce;

    class PrintTo;

    class ToVector;

    class Nth;

    /*
      Each trait _MUST_ be specialized for each terminator. 
    */
    namespace traits
    {
      template <class>
      struct supports_infinite {};

      template <class Term, class Op>
      struct supports_infinite<Compose<Term, Op>>;

    } // namespace traits
  } // namespace terminators

  /*
    Each class must be a Callable of type
      (Stream<A>&&) -> Stream<B>

    These classes are __NOT ALLOWED TO INTERACT WITH STREAM PROVIDER__
      to ensure that no calculations on stream occur
      until stream is terminated.

    All these classes can do in their operator()
      is to create streams from new instances of providers.

    If you want to implement new operator, you must either use existing
      providers or create new from scratch.
  */
  namespace operators {
    class Get;

    class Skip;

    template <class Transform>
    class Map;

    template <class Predicate>
    class Filter;

    class Group;

  } // namespace operators

  /*
    Public interface to use with streams.
  */

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

  /*
    Core stream class
  */
  template <class Provider>
  class Stream;

} // namespace stream
