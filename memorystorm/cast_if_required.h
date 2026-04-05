#pragma once

/// cast_if_required: static_cast a value to type T, only when T differs from the source type.
/// This avoids unnecessary cast warnings while still allowing narrowing conversions explicitly.

template<typename T, typename U>
inline T cast_if_required(U value) {
  return static_cast<T>(value);
}

template<typename T>
inline T cast_if_required(T value) {
  return value;
}
