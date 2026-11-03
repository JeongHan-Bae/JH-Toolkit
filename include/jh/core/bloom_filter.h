/**
 * @copyright
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo\@gmail.com&gt;
 * <br>
 * Licensed under the Apache License, Version 2.0 (the "License"); <br>
 * you may not use this file except in compliance with the License.<br>
 * You may obtain a copy of the License at<br>
 * <br>
 *     http://www.apache.org/licenses/LICENSE-2.0<br>
 * <br>
 * Unless required by applicable law or agreed to in writing, software<br>
 * distributed under the License is distributed on an "AS IS" BASIS,<br>
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.<br>
 * See the License for the specific language governing permissions and<br>
 * limitations under the License.<br>
 * <br>
 * Full license: <a href="https://github.com/JeongHan-Bae/JH-Toolkit?tab=Apache-2.0-1-ov-file#readme">GitHub</a>
 */
/**
 * @file bloom_filter.h
 * @brief Fixed-capacity Bloom filter using user-provided compile-time hashers.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 */

#pragma once

#include "jh/conceptual/tuple_like.h"
#include "jh/metax/flatten_proxy.h"
#include "jh/pods/array.h"
#include "jh/typing/monostate.h"

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <span>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

namespace jh {

    namespace detail {

        template<typename Result, std::size_t... I>
        consteval bool bloom_hash_elements(std::index_sequence<I...>) {
            return ((std::constructible_from<
                    std::size_t, decltype(jh::meta::detail::tuple_get_forwarded<I>(
                        std::declval<std::remove_cvref_t<Result> &>()))
            > && std::constructible_from<
                    std::remove_cvref_t<decltype(jh::meta::detail::tuple_get_forwarded<I>(
                        std::declval<std::remove_cvref_t<Result> &>()))>,
                    decltype(jh::meta::detail::tuple_get_forwarded<I>(
                        std::declval<std::remove_cvref_t<Result> &>()))
            >) && ...);
        }

        template<typename Result>
        consteval bool bloom_hash_result() {
            if constexpr (jh::concepts::tuple_like<Result>) {
                return bloom_hash_elements<Result>(
                    std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Result>>>{}
                );
            } else {
                return false;
            }
        }

        template<typename T, auto Hasher>
        concept bloom_hasher = requires(const T &value) {
            Hasher(value);
        } && bloom_hash_result<decltype(Hasher(std::declval<const T &>()))>();

        template<typename Allocator, typename = void>
        struct bloom_word_allocatable : std::false_type {};

        template<typename Allocator>
        struct bloom_word_allocatable<Allocator, std::void_t<
            typename Allocator::value_type,
            typename std::allocator_traits<Allocator>::template rebind_alloc<std::uint64_t>
        >> {
            using rebound = typename std::allocator_traits<Allocator>
                    ::template rebind_alloc<std::uint64_t>;
            using pointer = typename std::allocator_traits<rebound>::pointer;

            static constexpr bool value =
                std::constructible_from<rebound, const Allocator &> &&
                std::default_initializable<pointer> &&
                std::equality_comparable<pointer> &&
                requires(rebound &allocator, pointer allocation) {
                    { allocator.allocate(1) } -> std::same_as<pointer>;
                    allocator.deallocate(allocation, 1);
                    { std::to_address(allocation) } -> std::same_as<std::uint64_t *>;
                    std::allocator_traits<rebound>::construct(
                        allocator, std::to_address(allocation), std::uint64_t{0});
                    std::allocator_traits<rebound>::destroy(
                        allocator, std::to_address(allocation));
                };
        };

        template<typename Allocator, std::size_t Modulo>
        concept bloom_storage_policy =
            (std::same_as<Allocator, jh::typed::monostate> &&
             Modulo / 64 + (Modulo % 64 != 0) <=
                 jh::pod::max_pod_array_bytes / sizeof(std::uint64_t)) ||
            (!std::same_as<Allocator, jh::typed::monostate> &&
             Modulo / 64 + (Modulo % 64 != 0) <=
                 static_cast<std::size_t>(std::numeric_limits<std::ptrdiff_t>::max()) /
                     sizeof(std::uint64_t) &&
             bloom_word_allocatable<Allocator>::value);

        template<std::size_t WordCount, bool Inline>
        struct bloom_inline_words { using type = jh::typed::monostate; };

        template<std::size_t WordCount>
        struct bloom_inline_words<WordCount, true> {
            using type = jh::pod::array<std::uint64_t, WordCount>;
        };

        template<typename Allocator, bool Inline>
        struct bloom_word_owner;

        template<typename Allocator>
        struct bloom_word_owner<Allocator, true> {
            using allocator_type = jh::typed::monostate;
            using pointer = jh::typed::monostate;
        };

        template<typename Allocator>
        struct bloom_word_owner<Allocator, false> {
            using allocator_type = typename std::allocator_traits<Allocator>
                    ::template rebind_alloc<std::uint64_t>;
            using traits = std::allocator_traits<allocator_type>;
            using pointer = typename traits::pointer;

            static std::uint64_t *create(allocator_type &allocator, pointer &allocation,
                                         std::size_t count) {
                if (count > traits::max_size(allocator)) {
                    throw std::length_error("Bloom word count exceeds allocator max_size");
                }
                allocation = traits::allocate(allocator, count);
                std::size_t constructed = 0;
                try {
                    std::uint64_t *raw = std::to_address(allocation);
                    for (; constructed < count; ++constructed) {
                        traits::construct(allocator, raw + constructed, std::uint64_t{0});
                    }
                    return raw;
                } catch (...) {
                    std::uint64_t *raw = std::to_address(allocation);
                    for (std::size_t i = 0; i < constructed; ++i) {
                        traits::destroy(allocator, raw + i);
                    }
                    traits::deallocate(allocator, allocation, count);
                    allocation = pointer{};
                    throw;
                }
            }

            static void destroy(allocator_type &allocator, pointer allocation,
                                std::size_t count) noexcept {
                if (allocation != pointer{}) {
                    std::uint64_t *raw = std::to_address(allocation);
                    for (std::size_t i = 0; i < count; ++i) {
                        traits::destroy(allocator, raw + i);
                    }
                    traits::deallocate(allocator, allocation, count);
                }
            }
        };

    } // namespace detail

    /**
     * @brief Fixed-capacity probabilistic membership filter.
     *
     * @details
     * Hashers produce tuple-like groups of values convertible to <code>std::size_t</code>.
     * Each element must also be materializable by value through
     * <code>flatten_proxy::get&lt;I&gt;()</code>; an element exposed as an lvalue
     * reference therefore needs a copy constructor.
     * Each value selects one bit after conversion to <code>std::size_t</code>
     * and reduction modulo <code>Modulo</code>.
     * Hasher result elements that borrow storage must remain valid until the
     * filter call finishes. The filter retains only bits, not keys or hash
     * result objects.
     * A negative result means the value is absent; a positive result means it may
     * be present, assuming stable hash results, complete insertion of the keys
     * represented by the backing store, and the documented concurrency contract.
     * The filter cannot remove individual values or grow its bit capacity.
     * <br>
     * <p>With <code>jh::typed::monostate</code>, the bit array is stored inside
     * the filter, subject to its 16 KiB limit. Other allocators are rebound
     * to <code>std::uint64_t</code> and own a fixed-size external word sequence.
     * The filter is neither copyable nor movable; destruction must not race with
     * any operation. A shared mutex protects the entire bit array. Queries hold
     * a shared lock throughout all hasher calls and bit checks, so each query
     * observes a state before or after a complete insertion or clear.</p>
     * <p>Hashers used concurrently must themselves be safe for concurrent
     * invocation. Hasher calls must not reenter the same filter.</p>
     * <p>For persistence, choose Hashers whose raw results are stable for every
     * input across runs, processes, and target platforms before inserting
     * values. A matching sentinel hash on reload checks only that sentinel;
     * it cannot establish stability for other inputs. See
     * <code>jh::bloom_persistence</code> for the record and validation contract.</p>
     * <p>If the backing service retains keys across process restarts, restore
     * a current filter record or rebuild the filter from those keys before
     * using a negative result to skip a backing-store lookup. A new filter has
     * zeroed bits; for inputs producing hash bits, it would reject existing
     * keys until repopulated. A stale saved record can have the same effect for
     * keys added after that record was written.</p>
     * <p><code>visit_words()</code> and <code>replace_words()</code> expose the
     * logical word sequence with synchronization and validation. Callers may
     * use them for their own persistence format; the supplied
     * <code>jh::bloom_persistence</code> is an optional convenience layer.</p>
     * <p>The same shared mutex protects all word reads and commits. Queries
     * and word visits hold a shared lock through their full read; insertions,
     * clears, and whole-word replacement commit under an exclusive lock.
     * Under the C++ shared-mutex contract, a reader therefore observes a
     * complete state before or after a commit.
     * A whole-word replacement can overwrite an earlier insertion; coordinate
     * startup restoration with accepting new inserts.</p>
     *
     * @see jh::bloom_persistence
     * @tparam T Element type accepted by each hasher as <code>const T&amp;</code>.
     * @tparam Modulo Hash modulus and fixed bit capacity. Only inline storage
     *                is subject to the <code>jh::pod::array</code> size limit.
     * @tparam Allocator <code>jh::typed::monostate</code> or an allocator that
     *                   can be rebound to allocate the word sequence.
     * @tparam Hashers Compile-time hash function objects, each returning a tuple-like group.
     */
    template<typename T, std::size_t Modulo,
             typename Allocator = jh::typed::monostate, auto... Hashers>
    requires (Modulo > 0 &&
              detail::bloom_storage_policy<Allocator, Modulo> &&
              sizeof...(Hashers) > 0 && (detail::bloom_hasher<T, Hashers> && ...))
    class bloom_filter final {
    public:
        /// @brief Bit capacity of the filter.
        static constexpr std::size_t bit_count = Modulo;

        /// @brief Number of 64-bit words in the bit array.
        static constexpr std::size_t word_count = Modulo / 64 + (Modulo % 64 != 0);

        /// @brief Number of flattened hash values produced for one input.
        static constexpr std::size_t hash_count = std::tuple_size_v<
            jh::meta::flatten_proxy<
                decltype(std::tuple{Hashers(std::declval<const T &>())...}) &>>;

    private:
        static constexpr bool inline_storage = std::same_as<Allocator, jh::typed::monostate>;
        using owner = detail::bloom_word_owner<Allocator, inline_storage>;
        using allocator_type = typename owner::allocator_type;
        using allocation_pointer = typename owner::pointer;
        using inline_array_type = typename detail::bloom_inline_words<word_count, inline_storage>::type;

        struct word_mask {
            std::size_t index;
            std::uint64_t mask;
        };

        using plan_type = std::array<word_mask, hash_count>;

        [[no_unique_address]] allocator_type allocator_{};
        [[no_unique_address]] inline_array_type inline_words_{};
        [[no_unique_address]] allocation_pointer allocation_{};
        std::uint64_t *words_ = nullptr;
        mutable std::shared_mutex mutex_;

        void initialize() {
            if constexpr (inline_storage) {
                words_ = inline_words_.data;
            } else {
                words_ = owner::create(allocator_, allocation_, word_count);
            }
        }

        static void add_mask(plan_type &plan, std::size_t &used,
                             std::size_t hash) noexcept {
            const std::size_t bit = hash % Modulo;
            const std::size_t index = bit / 64;
            const std::uint64_t mask = std::uint64_t{1} << (bit % 64);
            for (std::size_t i = 0; i < used; ++i) {
                if (plan[i].index == index) {
                    plan[i].mask |= mask;
                    return;
                }
            }
            plan[used++] = {index, mask};
        }

        [[nodiscard]] bool has_bit(std::size_t hash) const noexcept {
            const std::size_t bit = hash % Modulo;
            return (words_[bit / 64] & (std::uint64_t{1} << (bit % 64))) != 0;
        }

        template<typename Proxy, std::size_t... I>
        static void add_group(plan_type &plan, std::size_t &used, const Proxy &proxy,
                              std::index_sequence<I...>) {
            (add_mask(plan, used, static_cast<std::size_t>(proxy.template get<I>())), ...);
        }

        template<typename Proxy, std::size_t... I>
        [[nodiscard]] bool check_group(const Proxy &proxy,
                                       std::index_sequence<I...>) const {
            return (has_bit(static_cast<std::size_t>(proxy.template get<I>())) && ...);
        }

        template<auto Hasher>
        [[nodiscard]] bool check_hasher(const T &value) const {
            auto hashes = Hasher(value);
            auto proxy = jh::meta::flatten_proxy{hashes};
            return check_group(proxy,
                std::make_index_sequence<std::tuple_size_v<decltype(proxy)>>{});
        }

    public:
        /**
         * @brief Constructs an empty filter with inline storage or a default allocator.
         */
        bloom_filter() requires std::default_initializable<allocator_type> {
            initialize();
        }

        /**
         * @brief Constructs an empty filter using the supplied allocator.
         * @param allocator Allocator rebound to the bit words.
         */
        explicit bloom_filter(const Allocator &allocator) requires (!inline_storage)
                : allocator_(allocator) {
            initialize();
        }

        bloom_filter(const bloom_filter &) = delete;
        bloom_filter &operator=(const bloom_filter &) = delete;
        bloom_filter(bloom_filter &&) = delete;
        bloom_filter &operator=(bloom_filter &&) = delete;

        /// @brief Destroys allocated storage after all concurrent operations have ended.
        ~bloom_filter() {
            if constexpr (!inline_storage) {
                owner::destroy(allocator_, allocation_, word_count);
            }
        }

        /**
         * @brief Inserts one value after all hash results have been planned.
         * @param value Value supplied to each hasher exactly once.
         *
         * If a hasher or hash conversion throws, the bit array is unchanged.
         */
        void insert(const T &value) {
            auto hashes = std::tuple{Hashers(value)...};
            auto proxy = jh::meta::flatten_proxy{hashes};
            plan_type plan{};
            std::size_t used = 0;
            add_group(plan, used, proxy,
                std::make_index_sequence<std::tuple_size_v<decltype(proxy)>>{});

            std::unique_lock lock(mutex_);
            for (std::size_t i = 0; i < used; ++i) {
                words_[plan[i].index] |= plan[i].mask;
            }
        }

        /**
         * @brief Checks whether all hash bits are set, one hasher at a time.
         * @param value Value supplied to each needed hasher.
         * @return False if definitely absent; true if possibly present.
         *
         * The call stops at the first missing bit and sees a coherent snapshot
         * relative to concurrent insertions and clears.
         */
        [[nodiscard]] bool possibly_contains(const T &value) const {
            std::shared_lock lock(mutex_);
            return (check_hasher<Hashers>(value) && ...);
        }

        /// @brief Clears every bit as one synchronized state change.
        void clear() {
            std::unique_lock lock(mutex_);
            if constexpr (inline_storage) {
                inline_words_ = {};
            } else {
                std::fill_n(words_, word_count, std::uint64_t{0});
            }
        }

        /**
         * @brief Visits each logical word in index order under one shared lock.
         * @tparam Visitor Callable accepting a word index and a word value.
         * @param visitor Function called once for each word.
         *
         * The callback receives values, so it cannot retain references to the
         * internal bit storage. It must not reenter this filter or wait for
         * another operation on the same filter: a queued writer may hold up
         * a later reader. A thrown exception releases the lock and leaves
         * the filter unchanged.
         */
        template<typename Visitor>
        requires std::invocable<Visitor &, std::size_t, std::uint64_t>
        void visit_words(Visitor &&visitor) const {
            std::shared_lock lock(mutex_);
            for (std::size_t i = 0; i < word_count; ++i) {
                visitor(i, std::uint64_t{words_[i]});
            }
        }

        /**
         * @brief Replaces all logical words after validating the input.
         * @param replacement Complete word sequence in logical index order.
         *
         * Throws <code>std::length_error</code> for a different word count and
         * <code>std::invalid_argument</code> for nonzero unused high bits.
         * Validation finishes before an exclusive lock commits the new state.
         * The caller keeps the replacement storage stable throughout this call.
         * This whole-state replacement may overwrite an earlier insertion.
         */
        void replace_words(std::span<const std::uint64_t> replacement) {
            if (replacement.size() != word_count) {
                throw std::length_error("Bloom replacement word count mismatch");
            }
            if constexpr (Modulo % 64 != 0) {
                constexpr std::uint64_t used =
                    (std::uint64_t{1} << (Modulo % 64)) - 1;
                if ((replacement.back() & ~used) != 0) {
                    throw std::invalid_argument("Bloom replacement has unused bits set");
                }
            }

            std::unique_lock lock(mutex_);
            std::copy(replacement.begin(), replacement.end(), words_);
        }
    };

} // namespace jh
