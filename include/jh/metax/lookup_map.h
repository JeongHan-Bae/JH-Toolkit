/**
 * \verbatim
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
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
 * \endverbatim
 */
/**
 * @file lookup_map.h (metax)
 * @brief constexpr and runtime fixed-size hash lookup table.
 * @author
 *   JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 *
 * <h3>Overview</h3>
 * <p>
 * Provides a sorted flat-map based on precomputed hash values. The table has
 * fixed capacity <tt>N</tt>, no allocation, deterministic layout, and allows
 * transparent key conversion. Lookup complexity is logarithmic by hash and
 * linear only within equal-hash ranges.
 * </p>
 *
 * <h4>Features</h4>
 * <ul>
 *   <li>constexpr construction</li>
 *   <li>transparent key transformation</li>
 *   <li>binary search on sorted hashes</li>
 *   <li>POD optimization into read-only memory</li>
 * </ul>
 */

#pragma once

#include <array>
#include <algorithm>
#include <type_traits>
#include <utility>
#include <concepts>
#include <string>
#include <string_view>
#include <cstddef>
#include "jh/pods/array.h"
#include "jh/pods/string_view.h"
#include "jh/conceptual/hashable.h"
#include "jh/metax/t_str.h"

namespace jh::meta {

    namespace extension {

        /**
         * @brief Default key conversion traits.
         *
         * @details
         * Defines canonical key type as <code>T</code> and accepts <code>const T&amp;</code>
         * as the apparent lookup input. Derived specializations may override conversion
         * rules to enable heterogeneous lookup.
         *
         * @tparam T Key canonical type.
         */
        template<typename T>
        struct key_traits {
            using canonical_type = T;
            using apparent_type = const T &;

            /**
             * @brief Convert apparent value to canonical key.
             *
             * @param v Input key reference.
             * @return Canonical key value.
             */
            static constexpr canonical_type to_canonical(apparent_type &v) noexcept {
                return v;
            }
        };

        /**
         * @brief Key traits specialization for <code>jh::pod::string_view</code>.
         *
         * @details
         * Supports conversion from several string-like forms to canonical POD view.
         *
         * @tparam none
         */
        template<>
        struct key_traits<jh::pod::string_view> {
            using canonical_type = jh::pod::string_view;

            using apparent_type = std::string_view;

            /**
             * @brief Convert char literal to canonical key.
             * @tparam N Literal length.
             * @param lit Null-terminated literal.
             */
            template<size_t N>
            static constexpr canonical_type to_canonical(const char (&lit)[N]) noexcept {
                return jh::pod::string_view::from_literal(lit);
            }

            /**
             * @brief Pass-through for canonical type.
             * @param v Canonical view.
             * @return Same object.
             */
            [[maybe_unused]] static constexpr canonical_type to_canonical(const canonical_type &v) noexcept {
                return v;
            }

            /**
             * @brief Convert std::string_view.
             * @param v Source view.
             */
            [[maybe_unused]] static constexpr canonical_type to_canonical(std::string_view v) noexcept {
                return {v.data(), v.size()};
            }

            /**
             * @brief Convert compile-time t_str literal.
             * @tparam N Size of t_str literal.
             * @param v Source t_str.
             */
            template<uint16_t N>
            [[maybe_unused]] static constexpr canonical_type to_canonical(const jh::meta::t_str<N> &v) noexcept {
                return {v.val(), v.size()};
            }

            /**
             * @brief Convert std::string.
             * @param v Source string.
             */
            [[maybe_unused]] static constexpr canonical_type to_canonical(const std::string &v) noexcept {
                return {v.data(), v.size()};
            }
        };

    } // namespace extension

    /**
     * @brief Concept checking whether key conversion through <code>key_traits&lt;K&gt;</code> is valid.
     *
     * @details
     * Enables heterogeneous lookup by verifying that an input type <code>T</code>
     * can be converted into the canonical key type <code>K</code> through
     * <code>key_traits&lt;K&gt;::to_canonical()</code>. This supports efficient
     * zero-overhead implicit transformation for lightweight or compile-time strings.
     *
     * <h4>Primary motivations:</h4>
     * <ul>
     *   <li>Lookup tables should not store heavyweight or runtime-managed content.</li>
     *   <li>Converted keys must remain low-cost and lifetime-stable.</li>
     *   <li><code>jh::pod::string_view</code> is recommended for string keys, usually via
     *       <code>"..."_psv</code>.</li>
     *   <li>Compile-time helpers can convert <code>std::string_view</code>
     *       (including <code>"..."sv</code> literals) into <code>jh::pod::string_view</code>
     *       safely.</li>
     *   <li>Design philosophy mirrors switch-like behavior: stable lifetime,
     *       direct literal use, and lightweight POD representations.</li>
     * </ul>
     *
     * @tparam K Canonical key type of the lookup table.
     * @tparam T Apparent input type to be checked for canonical conversion.
     *
     * @note This concept only checks convertibility; it does not validate semantics
     *       of the resulting canonical key.
     */
    template<typename K, typename T>
    concept transparent_key = requires(T t) {
        { jh::meta::extension::key_traits<K>::to_canonical(t) };
    };

    /**
     * @brief Fixed-capacity hash-based flat map providing switch-like lookup semantics.
     *
     * @details
     * Provides a switch-style lookup mechanism for types supporting hashing and equality.
     * Designed to deliver predictable performance across constexpr and runtime paths.
     *
     * <h4>Design motivations</h4>
     * <ul>
     *   <li>The lookup cost is <code>O(log(N))</code>, based on binary search over precomputed hashes.</li>
     *   <li>For small <code>N</code>, the dominant cost is computing the hash. If a type would require
     *       hashing before participating in a switch-like dispatch, the total cost closely
     *       matches a switch under small-table conditions.</li>
     *   <li>For large <code>N</code>, both this structure and a compiler-lowered switch typically operate
     *       in <code>O(log(N))</code>, so asymptotic behavior does not differ significantly.</li>
     *   <li>Poor hash quality is tolerated: the structure remains correct and performance
     *       degrades safely toward <code>O(N)</code>.</li>
     *   <li>Hash collisions are resolved by short linear scans inside the <tt>equal-hash</tt> range.</li>
     *   <li>The structure generalizes switch semantics to types that are not natively
     *       switchable, capturing the complete hash-then-dispatch pattern while ensuring
     *       deterministic layout and constexpr capability.</li>
     * </ul>
     *
     * <h4>Transparent lookup behavior</h4>
     * <ul>
     *   <li>Transparent <code>operator[]</code> queries are enabled through
     *       <code>jh::meta::extension::key_traits&lt;K&gt;</code>, which defines how an
     *       apparent input type <code>KeyIn</code> is converted into the canonical key
     *       type <code>K</code>.</li>
     *   <li>This conversion is explicit: <b>a canonical <code>K</code> object will be constructed</b>
     *       from the apparent input. Therefore <code>K</code> is expected to be lightweight,
     *       such as a POD key or a full-lifetime literal-based string view
     *       (<code>"..."_psv</code>).</li>
     *   <li>This differs from heterogeneous lookup in <code>unordered_map</code>; the goal here
     *       is predictable constexpr behavior, not amortized dynamic optimization.</li>
     * </ul>
     *
     * <h4>Implementation</h4>
     * <ol>
     *   <li>Entries are pre-hashed using <code>Hash</code> and stored as a fixed-size array.</li>
     *   <li>The array is sorted by hash, enabling binary search on the hash field.</li>
     *   <li>Equal-hash entries are resolved by a short linear comparison scan.</li>
     *   <li>Construction uses constexpr sorting when evaluated at compile time and
     *       runtime sorting otherwise.</li>
     *   <li>The container uses no dynamic allocation and provides deterministic layout.</li>
     * </ol>
     *
     * @tparam K Canonical key type stored in the map.
     * @tparam V Value type associated with each key.
     * @tparam N Number of stored entries.
     * @tparam Hash Hash functor producing <code>size_t</code>.
     *
     * @note Prefer compile-time construction with lightweight POD keys or full-lifetime
     *       string-view literals (e.g. <code>"..."_psv</code>) to ensure zero-overhead
     *       canonical conversions through <code>key_traits</code>.
     * </ul>
     */
    template<typename K, typename V, size_t N, typename Hash> requires requires(K k) {
        { Hash{}(k) } -> std::convertible_to<size_t>;
    }
    struct lookup_map {

        /**
         * @brief Single entry stored in the lookup table.
         *
         * @details
         * Contains precomputed hash, the canonical key, and its associated value.
         */
        struct entry {
            size_t hash; ///< Precomputed hash.
            K key;       ///< Canonical key.
            V value;     ///< Stored value.

            constexpr bool operator<(const entry &rhs) const noexcept { return hash < rhs.hash; }

            constexpr bool operator==(const entry &rhs) const noexcept { return hash == rhs.hash; }

            friend constexpr bool operator<(const entry &lhs, size_t rhs_hash) noexcept { return lhs.hash < rhs_hash; }

            friend constexpr bool operator<(size_t lhs_hash, const entry &rhs) noexcept { return lhs_hash < rhs.hash; }
        };

        static constexpr size_t entry_size = sizeof(entry);
        static constexpr size_t total_size = entry_size * N;

        /**
         * @brief Storage type selected based on POD suitability.
         *
         * @details If <code>entry</code> is POD-like and the total size does not exceed
         * <code>jh::pod::max_pod_array_bytes</code>, <code>jh::pod::array&lt;entry, N&gt;</code>
         * is used; otherwise <code>std::array&lt;entry, N&gt;</code> is selected.
         * <br>
         * Using the POD variant enables placement in read-only segments.
         */
        using container_type = decltype([]() {
            if constexpr (jh::pod::pod_like<entry> && (total_size <= jh::pod::max_pod_array_bytes))
                return std::type_identity<jh::pod::array<entry, N>>{};
            else
                return std::type_identity<std::array<entry, N>>{};
        }())::type;

        container_type entries;   ///< Sorted entries.
        V default_value;          ///< Value returned when key not found.

        /**
         * @brief Construct from an array of <code>std::pair&lt;K,V&gt;</code>.
         *
         * @details
         * Computes hashes, stores entries and sorts them by hash. Performs constexpr
         * or runtime sorting depending on evaluation context.
         *
         * @param init         Array of key-value pairs.
         * @param default_val  Value returned when lookup fails.
         * @param hasher       Hash functor.
         */
        template<typename Arr>
        requires(std::same_as<std::remove_cvref_t<Arr>, std::array<std::pair<K, V>, N>>)
        constexpr explicit lookup_map(Arr &&init,
                                      V default_val = {},
                                      Hash hasher = {})
                : entries({}), default_value(default_val) {
            if (std::is_constant_evaluated()) {
                for (size_t i = 0; i < N; ++i)
                    entries[i] = entry{hasher(init[i].first),
                                       (init[i].first),
                                       (init[i].second)};

                for (size_t i = 0; i < N; ++i)
                    for (size_t j = i + 1; j < N; ++j)
                        if (entries[j].hash < entries[i].hash)
                            std::swap(entries[i], entries[j]);
            } else {
                std::transform(init.begin(), init.end(), entries.begin(),
                               [&](auto &&p) {
                                   return entry{hasher(p.first),
                                                (p.first),
                                                (p.second)};
                               });
                std::sort(entries.begin(), entries.end(),
                          [](auto const &a, auto const &b) { return a.hash < b.hash; });
            }
        }

    private:
        /**
         * @brief Locate the first entry whose hash is not less than <code>h</code>.
         *
         * <p>
         * Uses constexpr binary search when evaluated at compile time, otherwise
         * <code>std::lower_bound</code>.
         * </p>
         */
        [[nodiscard]] constexpr size_t lower_bound_hash(size_t h) const noexcept {
            if (std::is_constant_evaluated()) {
                size_t l = 0, r = N;
                while (l < r) {
                    size_t mid = (l + r) / 2;
                    if (entries[mid].hash < h)
                        l = mid + 1;
                    else
                        r = mid;
                }
                return l;
            } else {
                auto it = std::lower_bound(
                        entries.begin(), entries.end(), h,
                        [](auto const &e, size_t hash_val) {
                            return e.hash < hash_val;
                        });
                return static_cast<size_t>(it - entries.begin());
            }
        }

    public:

        /**
         * @brief Lookup operator using transparent key conversion.
         *
         * @tparam KeyIn Input key type convertible via <code>key_traits&lt;K&gt;</code>.
         * @param key_in Key to query.
         * @return Reference to the stored value or <code>default_value</code> if not found.
         */
        template<typename KeyIn>
        requires transparent_key<K, KeyIn>
        constexpr const V &operator[](KeyIn &&key_in) const noexcept {
            using traits = jh::meta::extension::key_traits<K>;
            Hash hasher{};
            auto key = traits::to_canonical(std::forward<KeyIn>(key_in));
            size_t h = hasher(key);

            size_t pos = lower_bound_hash(h);
            for (size_t i = pos; i < N && entries[i].hash == h; ++i)
                if (entries[i].key == key)
                    return entries[i].value;

            return default_value;
        }

    };

    /**
     * @brief Deduction guide for constructing <code>lookup_map</code> from an array of pairs.
     *
     * @details
     * The hash functor deduced here is <code>jh::hash&lt;K&gt;</code>. This is not a fixed hash:
     * it is a dispatcher whose resolution order is:
     * <ol>
     *   <li><b>std::hash&lt;K&gt;</b> (always preferred if present)</li>
     *   <li>ADL-discovered <code>hash(K)</code></li>
     *   <li>member function <code>K::hash()</code></li>
     * </ol>
     *
     * <p>
     * Note that most <code>std::hash&lt;T&gt;</code> implementations in the standard library
     * are <b>not</b> constexpr. Therefore, when this deduction guide selects an
     * <code>std::hash&lt;K&gt;</code> that is not constexpr-friendly, compile-time evaluation of
     * <code>lookup_map</code> will fail. In such cases you must explicitly pass a constexpr hash.
     * </p>
     *
     * @tparam K Key type inferred from the pair array.
     * @tparam V Value type.
     * @tparam N Number of entries.
     */
    template<typename K, typename V, std::size_t N> requires jh::concepts::extended_hashable<K>
    lookup_map(std::array<std::pair<K, V>, N> &&)
    -> lookup_map<K, V, N, jh::hash<K>>;

    /**
     * @brief Deduction guide for constructing <code>lookup_map</code> with an explicit default value.
     *
     * @details
     * Uses <code>jh::hash&lt;K&gt;</code> as the hash dispatcher. If the selected hash source
     * (typically <code>std::hash&lt;K&gt;</code>) is not constexpr-capable, compile-time
     * instantiation cannot succeed. To construct a constexpr table, provide a custom
     * constexpr hash functor explicitly.
     *
     * @tparam K Key type.
     * @tparam V Value type.
     * @tparam N Number of entries.
     */
    template<typename K, typename V, std::size_t N> requires jh::concepts::extended_hashable<K>
    lookup_map(std::array<std::pair<K, V>, N> &&, V)
    -> lookup_map<K, V, N, jh::hash<K>>;

    /**
     * @brief Deduction guide for constructing <code>lookup_map</code> using a user-provided hash.
     *
     * @details
     * This overload bypasses the <code>jh::hash&lt;K&gt;</code> dispatcher entirely and uses
     * the caller-specified <code>Hash</code>. When constexpr operation is required, supplying
     * a constexpr-capable hash functor here is the recommended approach.
     *
     * @tparam Hash User-specified hash type.
     * @tparam K Key type.
     * @tparam V Value type.
     * @tparam N Entry count.
     */
    template<typename Hash, typename K, typename V, std::size_t N> requires requires(K k) {
        { Hash{}(k) } -> std::convertible_to<size_t>;
    }
    lookup_map(std::array<std::pair<K, V>, N> &&, V, Hash)
    -> lookup_map<K, V, N, Hash>;

    /**
     * @brief Compile-time constructor for <code>lookup_map</code> with explicit hash.
     *
     * @details
     * Requires <code>Hash</code> to be explicitly provided. This is the most reliable way
     * to construct a constexpr lookup table because no automatic hash deduction is used.
     * Use this overload when:
     * <ul>
     *   <li>the deduced <tt>jh::hash&lt;K&gt;</tt> is not constexpr, or</li>
     *   <li>full control of hashing behavior is required.</li>
     * </ul>
     *
     * <p>
     * Standard-library <code>std::hash&lt;T&gt;</code> is usually not constexpr. If deduction
     * would select such a hash, constexpr construction will fail; providing an explicit
     * constexpr hash functor avoids this issue.
     * </p>
     *
     * <h4>Usage example</h4>
     * @code
     * struct MyHash {
     *     constexpr size_t operator()(KeyType k) const noexcept {
     *         return ...; // constexpr-capable hashing
     *     }
     * };
     *
     * constexpr auto table =
     *     jh::meta::make_lookup_map&lt;MyHash&gt;(
     *         std::array{
     *             std::pair{ KeyType{...}, ValueType{...} },
     *             ...
     *             std::pair{ KeyType{...}, ValueType{...} }
     *         },
     *         ValueType{...});
     * @endcode
     *
     * @tparam Hash Hash functor used for all entries.
     * @tparam K Key type.
     * @tparam V Value type.
     * @tparam N Number of entries.
     *
     * @param init Key-value pairs.
     * @param default_value Value returned on lookup failure.
     * @param hash_fn Explicit hash functor instance.
     *
     * @return A fully constexpr lookup table.
     */
    template<typename Hash, typename K, typename V, std::size_t N>
    requires requires(K k) {
        { Hash{}(k) } -> std::convertible_to<size_t>;
    }
    consteval auto make_lookup_map(
            const std::array<std::pair<K, V>, N> &init,
            V default_value = V{},
            Hash hash_fn = Hash{}) {
        return lookup_map<K, V, N, Hash>{init, default_value, hash_fn};
    }

    /**
     * @brief Compile-time constructor using automatically deduced <code>jh::hash&lt;K&gt;</code>.
     *
     * @details
     * Deduces the hash functor as <code>jh::hash&lt;K&gt;</code>, which dispatches hashing by
     * prioritizing <code>std::hash&lt;K&gt;</code>. Since most standard-library hash
     * implementations are not constexpr, deduction may select a non-constexpr hash,
     * causing compile-time construction to fail. In such cases, the explicit-hash overload
     * must be used.
     *
     * <p>
     * This overload is safe when <code>K</code> itself provides a constexpr-capable hashing
     * mechanism. Types satisfying this condition include:
     * <ul>
     *   <li>user-defined types supplying a constexpr <code>std::hash&lt;K&gt;</code>, or</li>
     *   <li><code>jh::pod::string_view</code>, which supports constexpr hashing.</li>
     * </ul>
     * </p>
     *
     * <h4>Usage example</h4>
     * @code
     * using namespace jh::pod::literals;
     *
     * // KeyType = jh::pod::string_view, which provides constexpr hashing.
     * constexpr auto table =
     *     jh::meta::make_lookup_map(
     *         std::array{
     *             std::pair{ "..."_psv, ValueType{...} },
     *             ...
     *             std::pair{ "..."_psv, ValueType{...} }
     *         },
     *         ValueType{...});
     * @endcode
     *
     * @tparam K Key type satisfying <code>extended_hashable</code>.
     * @tparam V Value type.
     * @tparam N Entry count.
     *
     * @param init Key-value pairs.
     * @param default_value Fallback value.
     */
    template<typename K, typename V, std::size_t N>
    requires jh::concepts::extended_hashable<K>
    consteval auto make_lookup_map(
            const std::array<std::pair<K, V>, N> &init,
            V default_value = V{}) {
        return make_lookup_map<jh::hash<K>, K, V, N>(init, default_value);
    }

    /**
     * @brief Compile-time constructor for tables declared with <code>std::string_view</code> keys.
     *
     * @details
     * This overload exists because <code>std::hash&lt;std::string_view&gt;</code> is not
     * constexpr, which makes a compile-time table using <code>K = std::string_view</code>
     * invalid. To preserve a natural declaration syntax using <tt>"..."sv</tt> while still
     * supporting constexpr construction, this function converts all keys into
     * <code>jh::pod::string_view</code>, whose hashing is constexpr-capable and
     * whose literal-backed storage never dangles.
     *
     * <p>
     * Conceptually, this is a syntactic convenience: although the user writes keys as
     * <tt>std::string_view</tt>, the actual stored key type is
     * <code>jh::pod::string_view</code>. The conversion is performed via
     * <code>key_traits&lt;jh::pod::string_view&gt;</code>.
     * </p>
     *
     * <h4>Usage example</h4>
     * @code
     * using namespace std::literals;
     *
     * constexpr auto table =
     *     jh::meta::make_lookup_map(
     *         std::array{
     *             std::pair{ "..."sv, ValueType{...} },
     *             ...
     *             std::pair{ "..."sv, ValueType{...} }
     *         },
     *         ValueType{...});
     * // The table stores keys as jh::pod::string_view internally.
     * @endcode
     *
     * @tparam V Value type.
     * @tparam N Number of entries.
     *
     * @param init Key-value pairs whose keys are written as <code>std::string_view</code>.
     * @param default_value Fallback value for missing keys.
     */
    template<typename V, std::size_t N>
    consteval auto make_lookup_map(
            const std::array<std::pair<std::string_view, V>, N> &init,
            V default_value = V{}) {
        using K = jh::pod::string_view;
        using canonical_pair = std::pair<K, V>;

        std::array<canonical_pair, N> converted{};

        for (std::size_t i = 0; i < N; ++i) {
            converted[i] = canonical_pair{
                    jh::meta::extension::key_traits<K>::to_canonical(init[i].first),
                    init[i].second
            };
        }

        return make_lookup_map<K, V, N>(converted, default_value);
    }
} // namespace jh::meta
