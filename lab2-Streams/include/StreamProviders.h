#include <memory>

namespace stream
{
namespace providers
{
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


} // namespace providers

} // namespace stream
