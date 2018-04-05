#ifndef INCLUDE_STREAMTERMINATORS_H
#define INCLUDE_STREAMTERMINATORS_H

#include <stdexcept>
#include <iostream>

namespace stream {
  auto nth(std::size_t index) {
    return Terminator([=](auto&& stream) mutable {
      auto& source = stream.GetSource();
      for (std::size_t cntr = 0; cntr < index + 1; ++cntr)
        if (!source->advance())
          throw std::runtime_error("ERROR::stream::nth: Unexpected end of stream");
      return std::move(*source->get());
    });
  }

  template <class Accumulator, class IdentityFn>
  auto reduce(IdentityFn&& identityFn, Accumulator&& accum) {
    return Terminator([=](auto&& stream) mutable {
      auto id = std::forward<IdentityFn>(identityFn);
      auto acc = std::forward<Accumulator>(accum);
      auto& source = stream.GetSource();
      auto result = id(std::move(*source->get()));
      while (source->advance()) {
        result = acc(result, std::move(*source->get()));
      }
      return result;
    });
  }

  template <class Accumulator>
  auto reduce(Accumulator&& accum) {
    return reduce([](auto x){return x;}, std::forward<Accumulator>(accum));
  }

  auto sum() {
    return reduce([](auto x, auto y) { return x + y; });
  }

  auto print_to(std::ostream& os, const char* delimiter = " ") {
    return Terminator([&os, delimiter](auto&& stream) -> std::ostream& {
      auto& source = stream.GetSource();
      while(source->advance()) {
        os << std::move(*source->get()) << delimiter;
      }
      return os;
    });
  }

  auto to_vector() {
    return Terminator([=](auto&& stream) mutable {
      using T = typename std::remove_reference_t<decltype(stream)>::value_type;
      auto& source = stream.GetSource();
      std::vector<T> result;
      while(source->advance()) {
        result.emplace_back(std::move(*source->get()));
      }
      return result;
    });
  }
  
} // namespace stream

#endif