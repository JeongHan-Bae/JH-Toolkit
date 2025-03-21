# pragma once

#include <memory>  // NOLINT
#include <vector>
#include <algorithm>
#include <thread>
#include <cstring>  // NOLINT std::memset
#include <future>
#include <iostream>
#include "jh/data_sink.h"


#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
    #include <immintrin.h>  // Prefetch in x86 with SIMD
    #define PREFETCH(addr) _mm_prefetch(reinterpret_cast<const char*>(addr), _MM_HINT_T0)
#elif defined(__aarch64__) || defined(_M_ARM64)
#include <arm_neon.h>  // Include <arm_neon.h> for Apple
#define PREFETCH(addr) __builtin_prefetch(addr, 0, 3)
#else
    #define PREFETCH(addr)  // Do Nothing
#endif

#if defined(__aarch64__) || defined(_M_ARM64)

inline void fast_memset(std::uint8_t *ptr, const std::uint8_t value, const std::uint64_t count) {
    const uint8x16_t val = vdupq_n_u8(value);
    std::uint64_t i = 0;

    const auto addr = reinterpret_cast<std::uintptr_t>(ptr);
    std::uint64_t offset = -addr & 15;
    offset = std::min(offset, count);

    for (std::uint64_t i_ = 0; i_ < offset; ++i_) {
        ptr[i_] = value;
    }


    for (; i + 16 <= count; i += 16) {
        vst1q_u8(ptr + i, val);
    }

    for (; i < count; ++i) {
        ptr[i] = value;
    }
}

inline void fast_memset(std::uint16_t *ptr, const std::uint16_t value, const std::uint64_t count) {
    const __attribute__((aligned(16))) uint16x8_t val = vdupq_n_u16(value);
    std::uint64_t i = 0;
    for (; i + 8 <= count; i += 8) {
        vst1q_u16(ptr + i, val);
    }
    for (; i < count; ++i) {
        ptr[i] = value;
    }
}

inline void fast_reverse(uint64_t *input, const std::uint64_t n) {
    for (std::uint64_t i = 0; i < n / 2; i += 2) {
        const uint64x2_t v0 = vld1q_u64(input + i);
        const uint64x2_t v1 = vld1q_u64(input + n - i - 2);

        // Exchange v0 & v1
        vst1q_u64(input + i, vextq_u64(v1, v1, 1));
        vst1q_u64(input + n - i - 2, vextq_u64(v0, v0, 1));
    }
}


#else
#define fast_memset std::memset  // Still use Memset in x86
void fast_reverse(uint64_t* input, std::uint64_t n) {
    std::reverse(input, input + n);
}
#endif


namespace jh::radix_impl {
    inline void count_sort_uint8_t(std::uint8_t *input, const std::uint64_t n, const bool descending) {
        std::uint64_t count[256] = {};

        std::uint64_t i = 0;
        for (; i + 8 <= n; i += 8) {
            ++count[input[i + 0]];
            ++count[input[i + 1]];
            ++count[input[i + 2]];
            ++count[input[i + 3]];
            ++count[input[i + 4]];
            ++count[input[i + 5]];
            ++count[input[i + 6]];
            ++count[input[i + 7]];
        }
        // Handle the rest values
        for (; i < n; ++i) {
            ++count[input[i]];
        }

        std::uint64_t index = 0;
        if (!descending) {
            for (std::uint64_t i_ = 0; i_ < 256; ++i_) {
                if (count[i_] > 0) {
                    fast_memset(input + index, static_cast<std::uint8_t>(i_), count[i_]);
                    index += count[i_];
                }
            }
        } else {
            for (std::uint64_t i_ = 256; i_-- > 0;) {
                if (count[i_] > 0) {
                    fast_memset(input + index, static_cast<std::uint8_t>(i_), count[i_]);
                    index += count[i_];
                }
            }
        }
    }

    inline void count_sort_uint16_t(std::uint16_t *input, const std::uint64_t n, const bool descending) {
        std::uint64_t count[65536] = {};

        std::uint64_t i = 0;
        for (; i + 8 <= n; i += 8) {
            ++count[input[i + 0]];
            ++count[input[i + 1]];
            ++count[input[i + 2]];
            ++count[input[i + 3]];
            ++count[input[i + 4]];
            ++count[input[i + 5]];
            ++count[input[i + 6]];
            ++count[input[i + 7]];
        }
        // Handle the rest values
        for (; i < n; ++i) {
            ++count[input[i]];
        }

        std::uint64_t index = 0;
        if (!descending) {
            for (std::uint64_t i_ = 0; i_ < 65536; ++i_) {
                if (count[i_] > 0) {
                    fast_memset(input + index, static_cast<std::uint16_t>(i_), count[i_]);
                    index += count[i_];
                }
            }
        } else {
            for (std::uint64_t i_ = 65536; i_-- > 0;) {
                if (count[i_] > 0) {
                    fast_memset(input + index, static_cast<std::uint16_t>(i_), count[i_]);
                    index += count[i_];
                }
            }
        }
    }


    template<typename T>
        requires (std::is_same_v<T, std::uint8_t> ||
                  std::is_same_v<T, std::uint16_t> ||
                  std::is_same_v<T, std::uint32_t> ||
                  std::is_same_v<T, std::uint64_t>)
    constexpr std::uint64_t get_base() {
        if constexpr (std::is_same_v<T, std::uint8_t> || std::is_same_v<T, std::uint16_t>) {
            return 256;
        } else {
            return 65536;
        }
    }


#if defined(__aarch64__) || defined(_M_ARM64)

    template<std::uint32_t BLOCK_SIZE>
    void fast_emplace_back(data_sink<std::uint64_t, BLOCK_SIZE> *buckets, const std::uint64_t *vals,
                           const std::uint64_t count) {
        for (std::uint64_t i = 0; i < count; i += 4) {
            const uint64x2_t v0 = vld1q_u64(vals + i);
            const uint64x2_t v1 = vld1q_u64(vals + i + 2);
            buckets[vgetq_lane_u64(v0, 0) & 0xFFFF].emplace_back(vgetq_lane_u64(v0, 0));
            buckets[vgetq_lane_u64(v0, 1) & 0xFFFF].emplace_back(vgetq_lane_u64(v0, 1));
            buckets[vgetq_lane_u64(v1, 0) & 0xFFFF].emplace_back(vgetq_lane_u64(v1, 0));
            buckets[vgetq_lane_u64(v1, 1) & 0xFFFF].emplace_back(vgetq_lane_u64(v1, 1));
        }
    }
#else
#define fast_emplace_back(buckets, vals, count) \
for (std::uint64_t i = 0; i < count; ++i) { buckets[vals[i] & 0xFFFF].emplace_back(vals[i]); }
#endif

    template<std::uint32_t BLOCK_SIZE>
    void radix_sort_uint32_reverse(std::uint32_t *input, const std::uint64_t n, const bool descending) {
        constexpr std::uint64_t BASE = 65536; // 2^16
        std::array<jh::data_sink<std::uint32_t, BLOCK_SIZE>, BASE> buckets;

        // Only two 2 ^ 16 vals, first handle the Higher can save time (not low -> high, so called reverse)
        std::uint64_t i = 0;
        for (; i + 8 <= n; i += 8) {
            buckets[input[i + 0] >> 16].emplace_back(input[i + 0]);
            buckets[input[i + 1] >> 16].emplace_back(input[i + 1]);
            buckets[input[i + 2] >> 16].emplace_back(input[i + 2]);
            buckets[input[i + 3] >> 16].emplace_back(input[i + 3]);
            buckets[input[i + 4] >> 16].emplace_back(input[i + 4]);
            buckets[input[i + 5] >> 16].emplace_back(input[i + 5]);
            buckets[input[i + 6] >> 16].emplace_back(input[i + 6]);
            buckets[input[i + 7] >> 16].emplace_back(input[i + 7]);
        }
        // Handle the rest values
        for (; i < n; ++i) {
            buckets[input[i] >> 16].emplace_back(input[i]);
        }

        std::uint64_t index = 0;

        for (std::uint64_t i_ = 0; i_ < BASE; ++i_) {
            if (!buckets[i_].empty()) {
                // statistics for lower
                alignas(64) std::uint64_t count[BASE] = {};
                for (const auto &bucket = buckets[i_]; const auto &val: bucket) {
                    ++count[val & 0xFFFF];
                }

                for (std::uint64_t j = 0; j < BASE; ++j) {
                    std::fill_n(&input[index], count[j], static_cast<std::uint32_t>(i_ << 16 | j));
                    index += count[j];
                }
            }
        }

        if (descending) {
            std::reverse(input, input + n);
        }
    }

    template<std::uint32_t BLOCK_SIZE>
    void radix_sort_uint64_t_4_rounds(std::uint64_t *input, const std::uint64_t n, const bool descending) {
        constexpr std::uint64_t BASE = 65536; // 2^16
        std::array<data_sink<std::uint64_t, BLOCK_SIZE>, BASE> buckets;
        std::array<data_sink<std::uint64_t, BLOCK_SIZE>, BASE> temp;

        std::uint64_t i = 0;
        for (; i + 32 <= n; i += 32) {
            PREFETCH(reinterpret_cast<const char*>(&input[i + 32])); // Prefetch to avoid heavy cost
            fast_emplace_back(buckets.data(), input + i, 32);
        }
        for (; i < n; ++i) {
            buckets[input[i] & 0xFFFF].emplace_back(input[i]);
        }
        auto clear_task = std::async(std::launch::async, [&] {
            for (std::uint64_t b = 0; b < BASE; ++b) {
                temp[b].clear_reserve();
            }
        });
        clear_task.get();
        std::swap(buckets, temp);

        for (std::uint64_t b = 0; b < BASE; ++b) {
            for (const auto &bucket = temp[b]; const auto &val: bucket) {
                PREFETCH(reinterpret_cast<const char*>(&val));
                buckets[val >> 16 & 0xFFFF].emplace_back(val);
            }
        }
        clear_task = std::async(std::launch::async, [&] {
            for (std::uint64_t b = 0; b < BASE; ++b) {
                temp[b].clear_reserve();
            }
        });
        clear_task.get();
        std::swap(buckets, temp);

        for (std::uint64_t b = 0; b < BASE; ++b) {
            for (const auto &bucket = temp[b]; const auto &val: bucket) {
                PREFETCH(reinterpret_cast<const char*>(&val));
                buckets[val >> 32 & 0xFFFF].emplace_back(val);
            }
        }

        std::uint64_t index = 0;
        for (std::uint64_t b = 0; b < BASE; ++b) {
            for (const auto &bucket = buckets[b]; const auto &val: bucket) {
                input[index++] = val;
            }
        }

        // fast reverse to economize time in uint64_t
        if (descending) {
            fast_reverse(input, n);
        }
    }
}

namespace jh {
    template<typename T>
    concept RadixSortable = std::is_same_v<T, std::uint8_t> ||
                            std::is_same_v<T, std::uint16_t> ||
                            std::is_same_v<T, std::uint32_t> ||
                            std::is_same_v<T, std::uint64_t>;

    template<typename Container>
    concept HasDataAndSize = requires(Container c)
    {
        { c.data() } -> std::same_as<typename Container::value_type *>;
        { c.size() } -> std::convertible_to<std::uint64_t>;
    };

    constexpr std::uint32_t get_block_size(const std::uint64_t size) {
        if (!(size >> 15)) return 1024;
        if (!(size >> 18)) return 2048;
        if (!(size >> 21)) return 4096;
        if (!(size >> 24)) return 8192;
        if (!(size >> 27)) return 16384;
        return 32768;
    }

    template<typename Container>
    concept RadixCompatible = requires(Container c)
    {
        typename Container::value_type;
        { c.data() } -> std::same_as<typename Container::value_type *>;
        { c.size() } -> std::convertible_to<std::uint64_t>;
    } && RadixSortable<typename Container::value_type>;

    template<RadixCompatible Container>
    void radix_sort(Container &container, bool descending = false) {
        using T = typename Container::value_type;
        constexpr std::uint64_t PTR_SIZE = sizeof(void *);
        static_assert(PTR_SIZE >= 8, "System pointer size must be at least 64-bit");

        auto *data = container.data();
        auto size = static_cast<std::uint64_t>(container.size());

        if constexpr (std::is_same_v<T, std::uint8_t>) {
            radix_impl::count_sort_uint8_t(data, size, descending);
            return;
        }
        if constexpr (std::is_same_v<T, std::uint16_t>) {
            radix_impl::count_sort_uint16_t(data, size, descending);
            return;
        }

        if constexpr (std::is_same_v<T, std::uint32_t>) {
            switch (get_block_size(size)) {
                case 1024:
                    radix_impl::radix_sort_uint32_reverse<1024>(data, size, descending);
                    break;
                case 2048:
                    radix_impl::radix_sort_uint32_reverse<2048>(data, size, descending);
                    break;
                case 4096:
                    radix_impl::radix_sort_uint32_reverse<4096>(data, size, descending);
                    break;
                case 8192:
                    radix_impl::radix_sort_uint32_reverse<8192>(data, size, descending);
                    break;
                case 16384:
                    radix_impl::radix_sort_uint32_reverse<16384>(data, size, descending);
                    break;
                default:
                    radix_impl::radix_sort_uint32_reverse<32768>(data, size, descending);
                    break;
            }

            return;
        }

        if constexpr (std::is_same_v<T, std::uint64_t>) {
            switch (get_block_size(size)) {
                case 1024:
                    radix_impl::radix_sort_uint64_t_4_rounds<1024>(data, size, descending);
                    break;
                case 2048:
                    radix_impl::radix_sort_uint64_t_4_rounds<2048>(data, size, descending);
                    break;
                case 4096:
                    radix_impl::radix_sort_uint64_t_4_rounds<4096>(data, size, descending);
                    break;
                case 8192:
                    radix_impl::radix_sort_uint64_t_4_rounds<8192>(data, size, descending);
                    break;
                case 16384:
                    radix_impl::radix_sort_uint64_t_4_rounds<16384>(data, size, descending);
                    break;
                default:
                    radix_impl::radix_sort_uint64_t_4_rounds<32768>(data, size, descending);
                    break;
            }
        }
    }

    template<typename Container, typename T>
    concept SortableContainer = requires(Container c)
    {
        { c.data() } -> std::same_as<T *>;
        { c.size() } -> std::convertible_to<std::uint64_t>;
        { *c.begin() } -> std::same_as<T &>;
        requires std::same_as<decltype(c.end()), decltype(c.begin())>;
    };

    template<typename Container>
    concept SortableCompatible = requires(Container c)
    {
        typename Container::value_type;
        { c.data() } -> std::same_as<typename Container::value_type *>;
        { c.size() } -> std::convertible_to<std::uint64_t>;
        { *c.begin() } -> std::same_as<typename Container::value_type &>;
        requires std::same_as<decltype(c.end()), decltype(c.begin())>;
    };

    template<SortableCompatible Container>
    void uint_sort(Container &container, bool descending = false) {
        using T = typename Container::value_type;
        auto *data = container.data();
        const auto size = static_cast<std::uint64_t>(container.size());

        if constexpr (std::is_same_v<T, std::uint8_t> || std::is_same_v<T, std::uint16_t>) {
            jh::radix_sort(container, descending);
        } else if constexpr (std::is_same_v<T, std::uint32_t>) {
            if (!(size >> 25)) {
                if (descending) {
                    std::sort(container.begin(), container.end(), std::greater<T>());
                } else {
                    std::sort(container.begin(), container.end());
                }
            } else if (!(size >> 29)) {
                // 1 << 25 <= size < 1 << 29
                jh::radix_sort(container, descending);
            } else {
                if (descending) {
                    std::stable_sort(container.begin(), container.end(), std::greater<T>());
                } else {
                    std::stable_sort(container.begin(), container.end());
                }
            }
        } else if constexpr (std::is_same_v<T, std::uint64_t>) {
            if (!(size >> 25)) {
                if (descending) {
                    std::sort(container.begin(), container.end(), std::greater<T>());
                } else {
                    std::sort(container.begin(), container.end());
                }
            } else {
                if (descending) {
                    std::stable_sort(container.begin(), container.end(), std::greater<T>());
                } else {
                    std::stable_sort(container.begin(), container.end());
                }
            }
        }
    }
}
