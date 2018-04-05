#ifndef INCLUDE_STREAMOPERATORS_H
#define INCLUDE_STREAMOPERATORS_H

#include <stdexcept>
#include <type_traits>

#include "StreamProviders.h"

namespace stream {
  auto get(std::size_t n) {
    return Operator([=](auto&& stream) {
      using T = typename std::remove_reference_t<decltype(stream)>::value_type;
      return Stream<T>(std::move(
        std::make_unique<providers::Get<T>>(
          std::move(stream.GetSource()), n)));
    });
  }
  
} // namespace stream

#endif