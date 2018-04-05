#ifndef INCLUDE_STREAMOPERATORS_H
#define INCLUDE_STREAMOPERATORS_H

#include <stdexcept>
#include <type_traits>
#include <utility>

#include "StreamProviders.h"

namespace stream {
  auto get(std::size_t n) {
    return Operator([=](auto&& stream) mutable {
      using T = typename std::remove_reference_t<decltype(stream)>::value_type;
      return Stream<T>(std::move(
        std::make_unique<providers::Get<T>>(
          std::move(stream.GetSource()), n)));
    });
  }
  
  template <class Transform>
  auto map(Transform&& transform) {
    return Operator([=] (auto&& stream) mutable {
      using T = typename std::remove_reference_t<decltype(stream)>::value_type;
      using U = std::result_of_t<Transform(T)>;
      return Stream<U>(std::move(
        std::make_unique<providers::Map<T, Transform>>(
          std::move(stream.GetSource()), std::forward<Transform>(transform))));
    });
  }

  template <class Predicate>
  auto filter(Predicate&& predicate) {
    return Operator([=] (auto&& stream) mutable {
      using T = typename std::remove_reference_t<decltype(stream)>::value_type;
      return Stream<T>(std::move(
        std::make_unique<providers::Filter<T, Predicate>>(
          std::move(stream.GetSource()), std::forward<Predicate>(predicate))));
    });
  }

  auto skip(std::size_t amount) {
    return Operator([=] (auto&& stream) mutable {
      using T = typename std::remove_reference_t<decltype(stream)>::value_type;
      auto& source = stream.GetSource();
      for (size_t i = 0; i < amount; ++i)
        source->advance();
      return Stream<T>(std::move(source));
    });
  }

  auto group(std::size_t N) {
    return Operator([=] (auto&& stream) mutable {
      using T = typename std::remove_reference_t<decltype(stream)>::value_type;
      return Stream<std::vector<T>>(std::move(
        std::make_unique<providers::Group<T>>(
          std::move(stream.GetSource()), N)));
    });
  }

} // namespace stream

#endif