/**
* Copyright 2025 JeongHan-Bae <mastropseudo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file bits.h (pods)
 * @author JeongHan-Bae <mastropseudo@gmail.com>
 * @brief POD-compatible fixed-size bitflag storage (`jh::pod::bitflags<N>`)
 */


#pragma once
#include <cstdint>
#include <type_traits>
#include "array.h"

namespace jh::pod {
    /// @brief Maximum allowed size of a POD bitflags structure: 4KB (4096 bytes).
    inline constexpr std::uint16_t max_pod_bitflags_bytes = 4 * 1024;

    /**
     * @brief Integer concept used to constrain bitflag operations to standard unsigned types.
     */
    template<typename T>
    concept std_uint = std::is_same_v<T, std::uint8_t> ||
                       std::is_same_v<T, std::uint16_t> ||
                       std::is_same_v<T, std::uint32_t> ||
                       std::is_same_v<T, std::uint64_t>;
    template<std::uint16_t N>

    /// @brief True if the bitflag length is backed by a native integer type.
    constexpr bool is_native_bitflags = N == 8 || N == 16 || N == 32 || N == 64;

    /**
     * @brief Convert an unsigned integer into a little-endian byte array.
     *
     * @note Always little-endian, regardless of platform endianness.
     *
     * @tparam UInt Must be one of: uint8_t, uint16_t, uint32_t, uint64_t.
     * @param val The input value to encode.
     * @return `array<uint8_t, sizeof(UInt)>` encoded in little-endian order.
     */
    template<std_uint UInt>
    [[nodiscard]] constexpr auto uint_to_bytes(const UInt val) {
        // jh::pod::array is a pod_like type -> constexpr friendly
        // Inplace construction + RVO to eliminate for-loop cost
        if constexpr (std::is_same_v<UInt, std::uint8_t>) {
            return array<std::uint8_t, 1>{val};
        } else if constexpr (std::is_same_v<UInt, std::uint16_t>) {
            return array<std::uint8_t, 2>{
                static_cast<std::uint8_t>(val),
                static_cast<std::uint8_t>(val >> 8)
            };
        } else if constexpr (std::is_same_v<UInt, std::uint32_t>) {
            return array<std::uint8_t, 4>{
                static_cast<std::uint8_t>(val),
                static_cast<std::uint8_t>(val >> 8),
                static_cast<std::uint8_t>(val >> 16),
                static_cast<std::uint8_t>(val >> 24)
            };
        } else {
            return array<std::uint8_t, 8>{
                static_cast<std::uint8_t>(val),
                static_cast<std::uint8_t>(val >> 8),
                static_cast<std::uint8_t>(val >> 16),
                static_cast<std::uint8_t>(val >> 24),
                static_cast<std::uint8_t>(val >> 32),
                static_cast<std::uint8_t>(val >> 40),
                static_cast<std::uint8_t>(val >> 48),
                static_cast<std::uint8_t>(val >> 56)
            };
        }
    }

    /**
     * @brief Convert a little-endian byte array into an unsigned integer.
     *
     * @note Only valid for native sizes: 1, 2, 4, 8 bytes.
     *
     * @tparam N Byte count.
     * @param arr Input array.
     * @return Corresponding unsigned integer of size N * 8.
     */
    template<std::uint16_t N>
        requires (is_native_bitflags<N * 8>)
    [[nodiscard]] constexpr auto bytes_to_uint(const array<std::uint8_t, N> &arr) {
        if constexpr (N == 1) {
            return static_cast<std::uint8_t>(arr[0]);
        } else if constexpr (N == 2) {
            return static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(arr[0]) |
                static_cast<std::uint16_t>(arr[1]) << 8
            );
        } else if constexpr (N == 4) {
            return static_cast<std::uint32_t>(arr[0]) |
                   static_cast<std::uint32_t>(arr[1]) << 8 |
                   static_cast<std::uint32_t>(arr[2]) << 16 |
                   static_cast<std::uint32_t>(arr[3]) << 24;
        } else {
            return static_cast<std::uint64_t>(arr[0]) |
                   static_cast<std::uint64_t>(arr[1]) << 8 |
                   static_cast<std::uint64_t>(arr[2]) << 16 |
                   static_cast<std::uint64_t>(arr[3]) << 24 |
                   static_cast<std::uint64_t>(arr[4]) << 32 |
                   static_cast<std::uint64_t>(arr[5]) << 40 |
                   static_cast<std::uint64_t>(arr[6]) << 48 |
                   static_cast<std::uint64_t>(arr[7]) << 56;
        }
    }

    namespace detail {
#if defined(__clang__) || defined(__GNUC__)
        template<std_uint T>
        [[nodiscard]] constexpr std::uint16_t popcount(T value) noexcept {
            if constexpr (sizeof(T) <= sizeof(unsigned int)) {
                return static_cast<std::uint16_t>(__builtin_popcount(static_cast<unsigned int>(value)));
            } else if constexpr (sizeof(T) <= sizeof(unsigned long)) {
                return static_cast<std::uint16_t>(__builtin_popcountl(static_cast<unsigned long>(value)));
            } else {
                return static_cast<std::uint16_t>(__builtin_popcountll(static_cast<unsigned long long>(value)));
            }
        }
#else
#include <bit>
        template<typename T>
        [[nodiscard]] constexpr std::uint16_t popcount(T value) noexcept {
            return static_cast<std::uint16_t>(std::popcount(value));
        }
#endif
        /**
         * @brief Bitflags implementation using a native unsigned integer.
         *
         * @tparam T One of uint8_t, uint16_t, uint32_t, uint64_t.
         */
        template<std_uint T>
        struct bitflags_uint {
            T bits;

            static constexpr std::uint16_t size() {
                if constexpr (std::is_same_v<T, std::uint8_t>) return 8;
                if constexpr (std::is_same_v<T, std::uint16_t>) return 16;
                if constexpr (std::is_same_v<T, std::uint32_t>) return 32;
                return 64;
            }

            constexpr void clear() noexcept { bits = 0; }
            constexpr void set(std::uint16_t i) noexcept { bits |= static_cast<T>(1) << i; }
            constexpr void clear(std::uint16_t i) noexcept { bits &= ~(static_cast<T>(1) << i); }
            constexpr void flip(std::uint16_t i) noexcept { bits ^= static_cast<T>(1) << i; }

            constexpr void set_all() noexcept { *this = max(); }
            constexpr void reset_all() noexcept { clear(); }

            [[nodiscard]] constexpr bool has(std::uint16_t i) const noexcept {
                return (bits & static_cast<T>(1) << i) != 0;
            }

            constexpr bitflags_uint operator|(const bitflags_uint &rhs) const noexcept {
                auto res = *this;
                res.bits |= rhs.bits;
                return res;
            }

            constexpr bitflags_uint &operator|=(const bitflags_uint &rhs) noexcept {
                bits |= rhs.bits;
                return *this;
            }

            constexpr bitflags_uint operator&(const bitflags_uint &rhs) const noexcept {
                auto res = *this;
                res.bits &= rhs.bits;
                return res;
            }

            constexpr bitflags_uint &operator&=(const bitflags_uint &rhs) noexcept {
                bits &= rhs.bits;
                return *this;
            }

            constexpr bitflags_uint operator^(const bitflags_uint &rhs) const noexcept {
                auto res = *this;
                res.bits ^= rhs.bits;
                return res;
            }

            constexpr bitflags_uint &operator^=(const bitflags_uint &rhs) noexcept {
                bits ^= rhs.bits;
                return *this;
            }

            constexpr bitflags_uint operator~() const noexcept {
                return static_cast<bitflags_uint>(~bits);
            }


            /// @brief Inverts all bits in-place.
            constexpr void flip_all() noexcept { bits = ~bits; }

            [[nodiscard]] constexpr std::uint16_t count() const noexcept {
                return popcount(bits);
            }

            [[nodiscard]] static constexpr bitflags_uint max() noexcept {
                return static_cast<bitflags_uint>(~T{});
            }

            constexpr bool operator==(const bitflags_uint &rhs) const noexcept = default;
        };

        template<std::uint16_t NUM_BYTES>
        struct bitflags_bytes {
            std::uint8_t data[NUM_BYTES];

            static constexpr std::uint16_t size() { return NUM_BYTES * 8; }

            constexpr void clear() noexcept {
                *this = bitflags_bytes{}; // better than resetting with a for-loop
            }

            constexpr void set(const std::uint16_t bit) noexcept {
                data[bit / 8] |= static_cast<std::uint8_t>(1 << (bit % 8));
            }

            constexpr void clear(const std::uint16_t bit) noexcept {
                data[bit / 8] &= ~static_cast<std::uint8_t>(1 << (bit % 8));
            }

            constexpr void flip(const std::uint16_t bit) noexcept {
                data[bit / 8] ^= static_cast<std::uint8_t>(1 << (bit % 8));
            }

            [[nodiscard]] constexpr bool has(const std::uint16_t bit) const noexcept {
                return (data[bit / 8] & static_cast<std::uint8_t>(1 << (bit % 8))) != 0;
            }

            constexpr void set_all() noexcept { *this = max(); }
            constexpr void reset_all() noexcept { clear(); }

            constexpr bitflags_bytes operator|(const bitflags_bytes &rhs) const noexcept {
                bitflags_bytes out = *this;
                for (std::uint16_t i = 0; i < NUM_BYTES; ++i) out.data[i] |= rhs.data[i];
                return out;
            }

            constexpr bitflags_bytes &operator|=(const bitflags_bytes &rhs) noexcept {
                for (std::uint16_t i = 0; i < NUM_BYTES; ++i) data[i] |= rhs.data[i];
                return *this;
            }

            constexpr bitflags_bytes operator&(const bitflags_bytes &rhs) const noexcept {
                bitflags_bytes out = *this;
                for (std::uint16_t i = 0; i < NUM_BYTES; ++i) out.data[i] &= rhs.data[i];
                return out;
            }

            constexpr bitflags_bytes &operator&=(const bitflags_bytes &rhs) noexcept {
                for (std::uint16_t i = 0; i < NUM_BYTES; ++i) data[i] &= rhs.data[i];
                return *this;
            }

            constexpr bitflags_bytes operator^(const bitflags_bytes &rhs) const noexcept {
                bitflags_bytes out = *this;
                for (std::uint16_t i = 0; i < NUM_BYTES; ++i) out.data[i] ^= rhs.data[i];
                return out;
            }

            constexpr bitflags_bytes &operator^=(const bitflags_bytes &rhs) noexcept {
                for (std::uint16_t i = 0; i < NUM_BYTES; ++i) data[i] ^= rhs.data[i];
                return *this;
            }

            constexpr bitflags_bytes operator~() const noexcept {
                bitflags_bytes out;
                for (std::uint16_t i = 0; i < NUM_BYTES; ++i)
                    out.data[i] = static_cast<std::uint8_t>(~data[i]);
                return out;
            }

            /// @brief Inverts all bits in-place.
            constexpr void flip_all() noexcept {
                for (std::uint16_t i = 0; i < NUM_BYTES; ++i)
                    data[i] = static_cast<std::uint8_t>(~data[i]);
            }

            // Max Size in uint16_t, count <= Max Size
            [[nodiscard]] constexpr std::uint16_t count() const noexcept {
                std::uint16_t total = 0;
                for (std::uint16_t i = 0; i < NUM_BYTES; ++i)
                    total += popcount(data[i]);
                return total;
            }

            [[nodiscard]] static constexpr bitflags_bytes max() noexcept {
                bitflags_bytes out;
                /// should not write memset or will disrupt constexpr semantics (compiler will do memset itself)
                for (std::uint16_t i = 0; i < NUM_BYTES; ++i)
                    out.data[i] = 0xFF;
                return out;
            }

            constexpr bool operator==(const bitflags_bytes &rhs) const noexcept = default;
        };
    }


    /**
     * @brief POD-compatible fixed-size bitflags structure.
     *
     * @tparam N Number of bits. Must be divisible by 8 and ≤ 32'768.
     *
     * - For N = 8,16,32,64 → Backed by native unsigned integer.
     * - For all other valid sizes → Backed by a fixed-size byte array (little-endian layout).
     *
     * ### Properties:
     * - All operations are constexpr-compatible and noexcept.
     * - `.set()`, `.clear()`, `.flip()`, `.has()` are **unchecked** (no bounds checks).
     * - Bitwise operators (`|`, `&`, `^`, `~`) are supported.
     * - `.count()` returns the number of bits set.
     * - `.set_all()` / `.reset_all()` / `.flip_all()` work as expected.
     * - `.bits` for N = 8,16,32,64; `.data` for other valid sizes.
     *
     * ### Design Constraints:
     * - Fully inline layout, no heap or dynamic allocation.
     * - Cannot exceed 4KB total memory usage.
     * - No virtual functions, no RTTI, trivially copyable.
     *
     * @note This is a POD type. It is suitable for use in:
     * - Memory-mapped files or sockets
     * - `std::memcpy` or binary serialization
     * - constexpr and `static_assert` contexts
     *
     * @warning This structure is low-level by design. It assumes caller correctness.
     */
    template<std::uint16_t N>
        requires (N % 8 == 0 && N <= 8 * max_pod_bitflags_bytes)
    struct bitflags : detail::bitflags_bytes<N / 8> {
    }; /// @brief Fallback specialization for non-native bit sizes (e.g., 24, 120, etc).


    /// @brief Specialization for 8-bit bitflags.
    template<>
    struct bitflags<8> : detail::bitflags_uint<std::uint8_t> {
    };

    /// @brief Specialization for 16-bit bitflags.
    template<>
    struct bitflags<16> : detail::bitflags_uint<std::uint16_t> {
    };

    /// @brief Specialization for 32-bit bitflags.
    template<>
    struct bitflags<32> : detail::bitflags_uint<std::uint32_t> {
    };

    /// @brief Specialization for 64-bit bitflags.
    template<>
    struct bitflags<64> : detail::bitflags_uint<std::uint64_t> {
    };

    /**
     * @brief Serialize a `bitflags<N>` into a byte array (snapshot).
     *
     * @note The output is always little-endian, regardless of the host architecture.
     */
    template<std::uint16_t N>
        requires (N % 8 == 0)
    [[nodiscard]] constexpr array<std::uint8_t, N / 8> to_bytes(bitflags<N> f) {
        if constexpr (is_native_bitflags<N>) {
            return uint_to_bytes(f.bits);
        } else {
            return {f.data};
            // jh::pod::array uses a trivial constructor (compiler will call memcpy)
        }
    }

    /**
     * @brief Deserialize a `bitflags<N>` from a byte-array (snapshot).
     *
     * @note Only uses the array's content. Does not validate semantic correctness.
     */
    template<std::uint16_t N>
    constexpr bitflags<N> from_bytes(array<std::uint8_t, N / 8> arr) {
        if constexpr (is_native_bitflags<N>) {
            return static_cast<bitflags<N>>(detail::bitflags_uint{.bits = bytes_to_uint<N / 8>(arr)});
        } else {
            return {arr.data}; // pod types -> safe copy
        }
    }
}

