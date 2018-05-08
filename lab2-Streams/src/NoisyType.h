#include <iostream>

namespace util {

template <class T>
class CountedNoisy
{
public:
  CountedNoisy() {
    if (enabled)
      std::cout << "ctor " << id << '\n';
  }
  CountedNoisy(const CountedNoisy& other) {
    if (enabled)
      std::cout << "copy ctor " << id << " <- " << other.id << '\n';
  }
  CountedNoisy(CountedNoisy&& other) {
    if (enabled)
      std::cout << "move ctor " << id << " <- " << other.id << '\n';
  }
  CountedNoisy& operator=(const CountedNoisy& other) {
    if (enabled)
      std::cout << "copy asgn " << id << " <- " << other.id << '\n';
    return *this;
  }
  CountedNoisy& operator=(CountedNoisy&& other) {
    if (enabled)
      std::cout << "move asgn " << id << " <- " << other.id << '\n';
    return *this;
  }
  ~CountedNoisy() {
    if (enabled)
      std::cout << "dtor " << id << '\n';
  }
  size_t GetId() const {
    return id;
  }
  static void Mute() {
    CountedNoisy<T>::enabled = false;
  }
  static void Unmute() {
    CountedNoisy<T>::enabled = true;
  }

private:
  static thread_local bool enabled;
  static thread_local size_t lastId;
  size_t id = lastId++;
};

template <class T>
thread_local bool CountedNoisy<T>::enabled = true;

template <class T>
thread_local size_t CountedNoisy<T>::lastId = 0;

template <class T>
std::ostream& operator<<(std::ostream& os, const CountedNoisy<T>& value) {
  os << "id: " << value.GetId();
  return os;
}

template <class T>
class NoisyType : public CountedNoisy<NoisyType<T>>
{
public:
  NoisyType() {}
  NoisyType(const T& other) : value(other) {}
  NoisyType(T&& other) : value(std::move(other)) {}
  NoisyType& operator=(const T& other) {
    value = other;
    return *this;
  }
  NoisyType& operator=(T&& other) {
    value = std::move(other);
    return *this;
  }
  operator T() const { return value; }
  const T& Value() const { return value; }
  T& Value() { return value; }

private:
  T value;
};

template <class T>
std::ostream& operator<<(std::ostream& os, const NoisyType<T>& value) {
  os << value.Value() << " (";
  os << static_cast<const CountedNoisy<NoisyType<T>>&>(value) << ")";
  return os;
}

}  // namespace util
