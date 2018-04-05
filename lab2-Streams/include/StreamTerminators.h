#ifndef INCLUDE_STREAMTERMINATORS_H
#define INCLUDE_STREAMTERMINATORS_H

#include <stdexcept>

namespace stream {
  auto nth(std::size_t index) {
    return Terminator([=](auto&& stream) {
      auto& source = stream.GetSource();
      for (std::size_t cntr = 0; cntr < index; ++cntr)
        if (!source->advance())
          throw std::runtime_error("ERROR::stream::nth: Unexpected end of stream");
      return std::move(*source->get());
    });
  }

  
} // namespace stream

#endif