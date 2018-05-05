#ifndef INCLUDE_STREAMOPERATORS_H
#define INCLUDE_STREAMOPERATORS_H

namespace stream {

template <class> class Stream;

namespace operators {

/*
  Operator class must be Callable:
  template <class Provider>
  auto operator()(Stream<Provider>&&) -> Stream<...>

  If stream is empty, 
    EmptyStreamException must be thrown.
*/

class Get
{
public:
  Get(size_t amount) :
    amount(amount)
  {}

  template <class Provider>
  auto operator()(Stream<Provider>&& stream) {
    return Stream(
      providers::Get<Provider>(
        std::move(stream.GetProvider()),
        amount
      )
    );
  }

private:
  size_t amount;
};

class Skip
{
public:
  Skip(size_t amount) : 
    amount(amount)
  {}

  template <class Provider>
  auto operator()(Stream<Provider>&& stream) {
    auto& provider = stream.GetProvider();
    for (size_t i = 0; i < amount; ++i) {
      if (!provider.Advance())
        throw EmptyStreamException();
    }
    return Stream(std::move(stream));
  }

private:
  size_t amount;
};

template <class Transform>
class Map
{
public:
  Map(Transform&& transform) : 
    transform(std::forward<Transform>(transform))
  {}

  template <class Provider>
  auto operator()(Stream<Provider>&& stream) {
    return Stream(
      providers::Map<Provider, Transform>(
        std::move(stream.GetProvider()), 
        std::move(transform)
      )
    );
  }

private:
  Transform transform;
};

template <class Predicate>
class Filter
{
public:
  Filter(Predicate&& predicate) :
    predicate(std::forward<Predicate>(predicate))
  {}

  template <class Provider> 
  auto operator() (Stream<Provider>&& stream) {
    return Stream(
      providers::Filter<Provider, Predicate>(
        std::move(stream.GetProvider()),
        std::move(predicate)
      )
    );
  }

private:
  Predicate predicate;
};

class Group
{
public:
  Group(size_t size) : 
    size(size)
  {}

  template <class Provider>
  auto operator() (Stream<Provider>&& stream) {
    return Stream(
      providers::Group<Provider>(
        std::move(stream.GetProvider()),
        size
      )
    );
  }

private:
  size_t size;
};

} // namespace operators
} // namespace stream

#endif
