#ifndef INCLUDE_STREAMPROVIDERS_H
#define INCLUDE_STREAMPROVIDERS_H

#include <memory>
#include <type_traits>

namespace stream {

namespace providers {

template<class T>
class StreamProvider
{
public:
  virtual ~StreamProvider() = default;
  virtual bool advance() = 0;
  virtual std::shared_ptr<T> get() = 0;
};

template<class T, class Iter>
class Iterator : public StreamProvider<T>
{
public:
  Iterator(Iter begin, Iter end) :
    current(begin), end(end) {}

  virtual ~Iterator() = default;

  bool advance() override {
    ++current;
    return current != end;
  }

  std::shared_ptr<T> get() override {
    return std::make_shared<T>(std::move(*current));
  }

  Iter current;
  Iter end;
};

template<class Container>
class ContainerOwner : public StreamProvider<typename Container::value_type>
{
using InternalStreamProvider = 
  providers::Iterator<
    typename Container::value_type,
    typename Container::iterator>;

public:
  ContainerOwner(Container&& cont) :
    container(std::move(cont)),
    source(std::make_unique<InternalStreamProvider>(
      container.begin(), container.end())) {}

  bool advance() override {
    return source->advance();
  }

  std::shared_ptr<typename Container::value_type> get() override {
    return source->get();
  }

private:
  Container container;
  std::unique_ptr<InternalStreamProvider> source;
};

template<class Generator>
class Generate : public StreamProvider<std::result_of_t<Generator()>>
{
using T = std::result_of_t<Generator()>;

public:
  Generate(Generator&& generator):
    generator(std::forward<Generator>(generator)),
    current(std::make_shared<T>(generator())) {}

  bool advance() override {
    current = std::make_shared<T>(generator());
    return true;
  }

  std::shared_ptr<T> get() override {
    return current;
  }

private:
  Generator generator;
  std::shared_ptr<T> current;
};

template <class T>
class Get : public StreamProvider<T> 
{
public:
  Get(std::unique_ptr<StreamProvider<T>>&& source, std::size_t n):
    source(std::move(source)), n(n) {}

  bool advance() override {
    if (current < n - 1) {
      ++current;
      source->advance();
      return true;
    }
    return false;
  }

  std::shared_ptr<T> get() override {
    return source->get();
  }

private:
  std::unique_ptr<StreamProvider<T>> source;
  std::size_t n;
  std::size_t current = 0;
};

template <class T, class Transform>
class Map : public StreamProvider<std::result_of_t<Transform(T)>> 
{
public:
  Map(std::unique_ptr<StreamProvider<T>>&& source, Transform&& transform):
    source(std::move(source)), transform(std::forward<Transform>(transform)) {}

  bool advance() override {
    return source->advance();
  }

  std::shared_ptr<std::result_of_t<Transform(T)>> get() override {
    return std::make_shared<std::result_of_t<Transform(T)>>(transform(*source->get()));
  }

private:
  std::unique_ptr<StreamProvider<T>> source;
  Transform transform;
};

} // namespace providers

template <class T>
using StreamProviderPtr = std::unique_ptr<providers::StreamProvider<T>>;

} // namespace stream

#endif