/**
 * @copyright
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo\@gmail.com&gt;
 * <br>
 * Licensed under the Apache License, Version 2.0 (the "License"); <br>
 * you may not use this file except in compliance with the License.<br>
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
 * @file bloom_persistence.h
 * @brief Fixed-layout binary persistence for <code>jh::bloom_filter</code>.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 */

#pragma once

#include "jh/core/bloom_filter.h"
#include "jh/metax/t_str.h"
#include "jh/serialize_io/value_conversion.h"

#include <cstddef>
#include <cstdint>
#include <istream>
#include <limits>
#include <memory>
#include <ostream>
#include <span>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

namespace jh {

    namespace detail {
        template<typename BloomFilter>
        struct is_bloom_filter : std::false_type {};

        template<typename T, std::size_t Modulo, typename Allocator, auto... Hashers>
        struct is_bloom_filter<jh::bloom_filter<T, Modulo, Allocator, Hashers...>>
                : std::true_type {};
    }

    template<jh::meta::TStr Signature, typename BloomFilter>
    requires detail::is_bloom_filter<BloomFilter>::value
    class bloom_persistence;

    /**
     * @brief Persists a Bloom filter in a compile-time fixed binary layout.
     *
     * <p><code>Signature</code> is both the record identifier and the text used
     * to construct the sentinel value of <code>T</code>. Every Hasher runs on
     * that sentinel, and its raw results are stored and compared on reload.
     * The record encodes only semantic fields: signature bytes, raw sentinel
     * hashes, bit capacity, and logical bit words. Integers use fixed
     * little-endian <code>uint64_t</code> encoding. No allocator, pointer,
     * or C++ object representation is written.</p>
     *
     * <p>Choose Hashers with stable raw results for every input across runs,
     * processes, and target platforms before using persistence. A successful
     * write says nothing about that property. Matching sentinel hashes on
     * reload detect some mismatches, but cannot prove that hashes for other
     * inputs are stable. A mismatch is rejected before the filter changes;
     * a match is not a general safety or integrity guarantee. In particular,
     * do not rely on unspecified <code>std::hash</code> behavior for persisted
     * filters.</p>
     *
     * <p>The format has no CRC or other payload integrity check. Callers that
     * need CRC must compute and validate it around the binary record.</p>
     * <p>This is an optional fixed-layout helper. It uses the filter's public
     * <code>visit_words()</code> and <code>replace_words()</code> operations.
     * A caller can define another format through the same operations without
     * privileged access to filter storage.</p>
     *
     * <p>For a service whose backing keys survive a process restart, save a
     * filter state that covers those keys and restore it before using Bloom
     * negatives to skip backing-store lookups. If the record is missing,
     * invalid, or older than the durable keys, rebuild from the authoritative
     * keys or bypass the filter until it is ready. A newly constructed filter
     * starts with zeroed bits.</p>
     *
     * @tparam Signature Compile-time record identifier and sentinel text.
     * @tparam BloomFilter Concrete Bloom filter type.
     */
    template<jh::meta::TStr Signature, typename T, std::size_t Modulo,
             typename Allocator, auto... Hashers>
    requires jh::concepts::serializable<T>
    class bloom_persistence<
        Signature, jh::bloom_filter<T, Modulo, Allocator, Hashers...>
    > final {
        using filter_type = jh::bloom_filter<T, Modulo, Allocator, Hashers...>;

        static void write_word(std::ostream &out, std::uint64_t value) {
            char bytes[8]{};
            for (std::size_t i = 0; i < 8; ++i) {
                bytes[i] = static_cast<char>((value >> (8 * i)) & 0xff);
            }
            out.write(bytes, 8);
            if (!out) throw std::runtime_error("Bloom persistence write failed");
        }

        [[nodiscard]] static std::uint64_t read_word(std::istream &in) {
            char bytes[8]{};
            in.read(bytes, 8);
            if (!in) throw std::runtime_error("Bloom persistence read failed");
            std::uint64_t value = 0;
            for (std::size_t i = 0; i < 8; ++i) {
                value |= static_cast<std::uint64_t>(
                    static_cast<unsigned char>(bytes[i])) << (8 * i);
            }
            return value;
        }

        template<typename Action>
        static void with_sentinel(Action &&action) {
            const std::string sentinel_text = Signature.str();
            // The sentinel stays in this scope and is destroyed before its text.
            if constexpr (jh::concepts::detail::string_roundtrip<T>) {
                const T sentinel{sentinel_text};
                action(sentinel);
            } else {
                T sentinel{};
                jh::serio::deserialize(sentinel_text, sentinel);
                action(sentinel);
            }
        }

        template<typename Proxy, std::size_t... I>
        static void write_hashes(std::ostream &out, const Proxy &proxy,
                                 std::index_sequence<I...>) {
            (write_word(out, static_cast<std::uint64_t>(
                proxy.template get<I>())), ...);
        }

        template<typename Proxy, std::size_t... I>
        static void verify_hashes(std::istream &in, const Proxy &proxy,
                                  std::index_sequence<I...>) {
            (([&] {
                const auto actual = read_word(in);
                const auto expected = static_cast<std::uint64_t>(
                    proxy.template get<I>());
                if (actual != expected) {
                    throw std::runtime_error("Bloom sentinel hash mismatch");
                }
            }()), ...);
        }

    public:
        /// @brief Valid signature byte count, excluding the implicit terminator.
        static constexpr std::size_t signature_bytes = Signature.size();

        /// @brief Number of eight-byte words occupied by the padded signature.
        static constexpr std::size_t signature_words =
            signature_bytes / 8 + (signature_bytes % 8 != 0);

        /// @brief Total number of raw hash values produced by all hashers.
        static constexpr std::size_t hash_count = filter_type::hash_count;

        /// @brief Number of bit storage words in the Bloom filter.
        static constexpr std::size_t word_count = filter_type::word_count;

        static_assert(std::numeric_limits<std::size_t>::digits <=
                      std::numeric_limits<std::uint64_t>::digits,
                      "Bloom persistence requires size_t to fit in uint64_t");
        static_assert(std::in_range<std::uint64_t>(signature_bytes));
        static_assert(std::in_range<std::uint64_t>(hash_count));
        static_assert(std::in_range<std::uint64_t>(Modulo));
        static_assert(signature_words <= std::numeric_limits<std::size_t>::max() / 8 - 3);
        static_assert(word_count <=
                      std::numeric_limits<std::size_t>::max() / 8 - 3 - signature_words);
        static_assert(hash_count <=
                      std::numeric_limits<std::size_t>::max() / 8 - 3 - signature_words - word_count);

        /// @brief Exact number of bytes in one serialized filter record.
        static constexpr std::size_t serialized_size =
            8 * (3 + signature_words + hash_count + word_count);

        /**
         * @brief Writes one fixed-length record to a binary output stream.
         * @param out Destination stream.
         * @param filter Filter to snapshot and serialize.
         *
         * Throws on invalid sentinel conversion or stream failure. The word
         * sequence is written under a shared lock for a coherent record.
         */
        static void serialize(std::ostream &out, const filter_type &filter) {
            with_sentinel([&](const T &sentinel) {
                auto hashes = std::tuple{Hashers(sentinel)...};
                auto proxy = jh::meta::flatten_proxy{hashes};
                static_assert(std::tuple_size_v<decltype(proxy)> == hash_count);

                write_word(out, static_cast<std::uint64_t>(signature_bytes));
                for (std::size_t i = 0; i < signature_words * 8; ++i) {
                    out.put(i < signature_bytes ? Signature.val()[i] : '\0');
                }
                if (!out) throw std::runtime_error("Bloom persistence signature write failed");

                write_word(out, static_cast<std::uint64_t>(hash_count));
                write_hashes(out, proxy, std::make_index_sequence<hash_count>{});
                write_word(out, static_cast<std::uint64_t>(Modulo));
                filter.visit_words([&](std::size_t, std::uint64_t word) {
                    write_word(out, word);
                });
            });
        }

        /**
         * @brief Reads, validates, then atomically restores one fixed-length record.
         * @param in Source stream positioned at the record.
         * @param filter Filter to replace only after every field passes validation.
         *
         * Leaves the filter unchanged on any validation or input failure.
         * Later records in the stream are not consumed.
         * A successful restore replaces the whole word state and can overwrite
         * insertions completed before its final commit.
         */
        static void deserialize(std::istream &in, filter_type &filter) {
            if (read_word(in) != static_cast<std::uint64_t>(signature_bytes)) {
                throw std::runtime_error("Bloom signature length mismatch");
            }
            for (std::size_t i = 0; i < signature_words * 8; ++i) {
                char byte{};
                in.get(byte);
                if (!in) throw std::runtime_error("Bloom signature read failed");
                const char expected = i < signature_bytes ? Signature.val()[i] : '\0';
                if (byte != expected) {
                    throw std::runtime_error("Bloom signature or padding mismatch");
                }
            }
            if (read_word(in) != static_cast<std::uint64_t>(hash_count)) {
                throw std::runtime_error("Bloom hash count mismatch");
            }

            with_sentinel([&](const T &sentinel) {
                auto hashes = std::tuple{Hashers(sentinel)...};
                auto proxy = jh::meta::flatten_proxy{hashes};
                static_assert(std::tuple_size_v<decltype(proxy)> == hash_count);
                verify_hashes(in, proxy, std::make_index_sequence<hash_count>{});

                if (read_word(in) != static_cast<std::uint64_t>(Modulo)) {
                    throw std::runtime_error("Bloom bit capacity mismatch");
                }
                std::unique_ptr<std::uint64_t[]> restored{new std::uint64_t[word_count]};
                for (std::size_t i = 0; i < word_count; ++i) {
                    restored[i] = read_word(in);
                }
                if constexpr (Modulo % 64 != 0) {
                    constexpr std::uint64_t used =
                        (std::uint64_t{1} << (Modulo % 64)) - 1;
                    if ((restored[word_count - 1] & ~used) != 0) {
                        throw std::runtime_error("Bloom unused bits are set");
                    }
                }

                filter.replace_words(
                    std::span<const std::uint64_t>{restored.get(), word_count}
                );
            });
        }
    };

    /**
     * @brief Serializes a filter while deducing its complete template type.
     * Uses the signature, sentinel, and hash-stability contract of
     * <code>jh::bloom_persistence</code>.
     * @tparam Signature Compile-time signature and sentinel text.
     * @param out Destination stream.
     * @param filter Filter to serialize.
     * @see jh::bloom_persistence
     */
    template<jh::meta::TStr Signature, typename T, std::size_t Modulo,
             typename Allocator, auto... Hashers>
    requires jh::concepts::serializable<T>
    void serialize_bloom(std::ostream &out,
                         const jh::bloom_filter<T, Modulo, Allocator, Hashers...> &filter) {
        bloom_persistence<Signature,
            jh::bloom_filter<T, Modulo, Allocator, Hashers...>>::serialize(out, filter);
    }

    /**
     * @brief Deserializes a filter while deducing its complete template type.
     * Uses the field validation and sentinel comparison contract of
     * <code>jh::bloom_persistence</code>.
     * @tparam Signature Compile-time signature and sentinel text.
     * @param in Source stream.
     * @param filter Filter to restore.
     * @see jh::bloom_persistence
     */
    template<jh::meta::TStr Signature, typename T, std::size_t Modulo,
             typename Allocator, auto... Hashers>
    requires jh::concepts::serializable<T>
    void deserialize_bloom(std::istream &in,
                           jh::bloom_filter<T, Modulo, Allocator, Hashers...> &filter) {
        bloom_persistence<Signature,
            jh::bloom_filter<T, Modulo, Allocator, Hashers...>>::deserialize(in, filter);
    }

} // namespace jh
