#ifndef LAB2_STREAMS_INCLUDE_STREAMINTERFACE_H_
#define LAB2_STREAMS_INCLUDE_STREAMINTERFACE_H_

#include <utility>

#include "StreamTerminators.h"
#include "StreamOperators.h"
#include "StreamOperations.h"

namespace stream {

// ---------------------------------------------------------
//  Terminators interface
// ---------------------------------------------------------

template <class Accumulator>
auto reduce(Accumulator&& accum) {
  return Terminator(
    terminators::Reduce(
      [](auto&& x) -> auto&& { return x; },
      std::forward<Accumulator>(accum)));
}

template <class IdentityFn, class Accumulator>
auto reduce(IdentityFn&& identityFn, Accumulator&& accum) {
  return Terminator(
    terminators::Reduce<IdentityFn, Accumulator>(
      std::forward<IdentityFn>(identityFn),
      std::forward<Accumulator>(accum)));
}

auto sum() {
  return Terminator(
    terminators::Reduce(
      [](auto&& x) -> auto&& { return x; },
      [](auto&& x, auto&& y) { return x + y; }));
}

auto print_to(std::ostream& os, const char* delimiter = " ") {
  return Terminator(
    terminators::PrintTo(os, delimiter));
}

auto to_vector() {
  return Terminator(
    terminators::ToVector());
}

auto nth(size_t index) {
  return Terminator(
    terminators::Nth(index));
}

// ---------------------------------------------------------
//  Operators interface
// ---------------------------------------------------------

auto get(size_t n) {
  return Operator(
    operators::Get(n));
}

template <class Transform>
auto map(Transform&& transform) {
  return Operator(
    operators::Map<Transform>(
      std::forward<Transform>(transform)));
}

template <class Predicate>
auto filter(Predicate&& predicate) {
  return Operator(
    operators::Filter<Predicate>(
      std::forward<Predicate>(predicate)));
}

auto skip(size_t amount) {
  return Operator(
    operators::Skip(amount));
}

auto group(size_t size) {
  return Operator(
    operators::Group(size));
}

}  // namespace stream

#endif  // LAB2_STREAMS_INCLUDE_STREAMINTERFACE_H_
