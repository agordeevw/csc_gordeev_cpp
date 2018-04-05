#ifndef INCLUDE_STREAMTERMINATORS_H
#define INCLUDE_STREAMTERMINATORS_H

#include <stdexcept>

namespace stream {
  auto nth(std::size_t index) {
    return Terminator([=](auto&& stream) mutable {
      auto& source = stream.GetSource();
      for (std::size_t cntr = 0; cntr < index; ++cntr)
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
  
} // namespace stream

#endif