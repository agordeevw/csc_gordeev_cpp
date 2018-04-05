#include <memory>

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

} // namespace providers

template <class T>
using StreamProviderPtr = std::unique_ptr<providers::StreamProvider<T>>;

} // namespace stream
