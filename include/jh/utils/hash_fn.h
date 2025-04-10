#pragma once

#include <cstdint>

namespace jh::utils::hash_fn {

    constexpr std::uint64_t fnv1a64(const char *data, const std::uint64_t size) noexcept {
           std::uint64_t h = 14695981039346656037ull;
           for (std::uint64_t i = 0; i < size; ++i) {
               h ^= static_cast<std::uint8_t>(data[i]);
               h *= 1099511628211ull;
           }
           return h;
       }
}