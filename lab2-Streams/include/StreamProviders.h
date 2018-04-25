#ifndef INCLUDE_STREAMPROVIDERS_H
#define INCLUDE_STREAMPROVIDERS_H

#include <memory>
#include <type_traits>
#include <list>

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
    if (first) {
      first = false;
      return current != end;
    }
    ++current;
    return current != end;
  }

  std::shared_ptr<T> get() override {
    return std::make_shared<T>(std::move(*current));
  }

  bool first = true;
  Iter current;
  Iter end;
};

template<class Container, class InternalStreamProvider = 
    providers::Iterator<
    typename Container::value_type,
    typename Container::iterator>>
class ContainerOwner : public StreamProvider<typename Container::value_type>
{
public:
  ContainerOwner(Container&& cont) :
    container(std::move(cont)),
    source(std::make_unique<InternalStreamProvider>(
      container.begin(), container.end())) {}

  virtual ~ContainerOwner() = default;

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
    generator(std::forward<Generator>(generator)) {}

  virtual ~Generate() = default;

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

  virtual ~Get() = default;

  bool advance() override {
    if (current < n) {
      ++current;
      return source->advance();
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

  virtual ~Map() = default;

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

template <class T, class Predicate>
class Filter : public StreamProvider<T>
{
public:
  Filter(std::unique_ptr<StreamProvider<T>>&& source, Predicate&& predicate):
    source(std::move(source)), predicate(std::forward<Predicate>(predicate)) {}

  virtual ~Filter() = default;

  bool advance() override {
    while (source->advance()) {
      current = source->get();
      if (predicate(*current))
        return true;
    }
    current.reset();
    return false;
  }

  std::shared_ptr<T> get() override {
    return current;
  }

private:
  std::unique_ptr<StreamProvider<T>> source;
  std::shared_ptr<T> current;
  Predicate predicate;
  bool first = true;
};

template <class T>
class Group : public StreamProvider<std::vector<T>>
{
public:
  Group(std::unique_ptr<StreamProvider<T>>&& source, size_t group_size) :
    source(std::move(source)), group_size(group_size) {}

  virtual ~Group() = default;

  bool advance() override {
    if (stream_ended) {
      current.reset();
      return false;
    }
    current = std::make_shared<std::vector<T>>();
    for (size_t i = 0; i < group_size; ++i) {
      if (source->advance())
        current->emplace_back(std::move(*source->get()));
      else {
        stream_ended = true;
        break;
      }
    }
    return true;
  }

  std::shared_ptr<std::vector<T>> get() override {
    return current;
  }

private:
  std::unique_ptr<StreamProvider<T>> source;
  std::shared_ptr<std::vector<T>> current;
  size_t group_size;
  bool stream_ended = false;
};

} // namespace providers

template <class T>
using StreamProviderPtr = std::unique_ptr<providers::StreamProvider<T>>;

} // namespace stream

#endif