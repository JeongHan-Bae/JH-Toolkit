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
 * @file huffman.h (serialize_io)
 * @author JeongHan-Bae &lt;mastropseudo&#64;gmail.com&gt;
 * @brief High-performance Huffman encoder/decoder supporting both canonical and
 * standard tree-based algorithms, with selectable symbol ranges (128 or 256).
 *
 * <p>
 * The <code>jh::serio::huffman</code> component provides four distinct
 * compression algorithms:
 * </p>
 *
 * <ul>
 *   <li><code>huff128</code> — Standard Huffman for ASCII (0–127)</li>
 *   <li><code>huff256</code> — Standard Huffman for full byte range (0–255)</li>
 *   <li><code>huff128_canonical</code> — Canonical Huffman (ASCII)</li>
 *   <li><code>huff256_canonical</code> — Canonical Huffman (full byte range)</li>
 * </ul>
 *
 * <p>
 * Canonical algorithms generate deterministic codewords, enabling extremely fast
 * decoding via table lookups without tree traversal. Normal Huffman algorithms
 * preserve legacy tree behavior for compatibility with older formats.
 * </p>
 *
 * <h3>Feature Comparison</h3>
 * <table>
 *   <tr>
 *     <th><nobr>Feature</nobr></th>
 *     <th><nobr><code>huff128</code></nobr></th>
 *     <th><nobr><code>huff128_canonical</code></nobr></th>
 *     <th><nobr><code>huff256</code></nobr></th>
 *     <th><nobr><code>huff256_canonical</code></nobr></th>
 *     </tr>
 *   <tr>
 *     <td>Symbol range</td>
 *     <td>0–127</td>
 *     <td>0–127</td>
 *     <td>0–255</td>
 *     <td>0–255</td>
 *   </tr>
 *   <tr>
 *     <td>Encoding speed</td>
 *     <td>&#10004; Fast</td>
 *     <td>&#10004; Fast<br>(minor setup cost)</td>
 *     <td>&#10004; Fast</td>
 *     <td>&#10004; Fastest overall</td>
 *   </tr>
 *   <tr>
 *     <td>Decoding speed</td>
 *     <td>&#10008; Slow<br>(tree traversal)</td>
 *     <td>&#10004;&#10004; Very fast<br>(table-based)</td>
 *     <td>&#10008; Slow<br>(tree traversal)</td>
 *     <td>&#10004;&#10004; Very fast<br>(table-based)</td>
 *   </tr>
 *   <tr>
 *     <td>Worst-case decode complexity</td>
 *     <td>O(depth &times; N)</td>
 *     <td>O(N)</td>
 *     <td>O(depth &times; N)</td>
 *     <td>O(N)</td>
 *   </tr>
 *   <tr>
 *     <td>Codeword determinism</td>
 *     <td>&#10008;</td>
 *     <td>&#10004; Canonical</td>
 *     <td>&#10008;</td>
 *     <td>&#10004; Canonical</td>
 *   </tr>
 *   <tr>
 *     <td>Best usage</td>
 *     <td>Legacy formats / debugging</td>
 *     <td>High-performance ASCII</td>
 *     <td>General binary data</td>
 *     <td>High-performance binary</td>
 *   </tr>
 * </table>
 *
 * <h3>Measured Performance</h3>
 * <p>
 * Apple Silicon M3, LLVM clang++ 20, input size 20k–200k bytes (2025).
 * </p>
 *
 * <table>
 *   <tr>
 *     <th><nobr>Operation</nobr></th>
 *     <th><nobr><code>huff128</code></nobr></th>
 *     <th><nobr><code>huff128_canonical</code></nobr></th>
 *     <th><nobr><code>huff256</code></nobr></th>
 *     <th><nobr><code>huff256_canonical</code></nobr></th>
 *   </tr>
 *   <tr><td colspan="5"><b>Debug (unoptimized)</b></td></tr>
 *   <tr>
 *     <td>Encode</td>
 *     <td>0.45–4.3 ms</td>
 *     <td>0.44–4.6 ms</td>
 *     <td>0.54–4.0 ms</td>
 *     <td>0.48–3.8 ms</td>
 *   </tr>
 *   <tr>
 *     <td>Decode</td>
 *     <td>1.4–15 ms</td>
 *     <td>0.39–3.8 ms</td>
 *     <td>1.5–15 ms</td>
 *     <td>0.44–4.2 ms</td>
 *   </tr>
 *   <tr>
 *     <td>Relative decode speed</td>
 *     <td>1&times; (baseline)</td>
 *     <td>&asymp; 3.5–4&times; faster</td>
 *     <td>1&times; (baseline)</td>
 *     <td>&asymp; 3.5–4&times; faster</td>
 *   </tr>
 *   <tr><td colspan="5"><b>O3 optimized</b></td></tr>
 *   <tr>
 *     <td>Encode</td>
 *     <td>0.155–1.51 ms</td>
 *     <td>0.154–1.49 ms</td>
 *     <td>0.156–1.52 ms</td>
 *     <td>0.156–1.48 ms</td>
 *   </tr>
 *   <tr>
 *     <td>Decode</td>
 *     <td>0.406–2.27 ms</td>
 *     <td>0.290–2.17 ms</td>
 *     <td>0.265–2.27 ms</td>
 *     <td>0.260–2.16 ms</td>
 *   </tr>
 *   <tr>
 *     <td>Relative decode speed</td>
 *     <td>1&times; (baseline)</td>
 *     <td>&asymp;1.05–1.4&times; faster</td>
 *     <td>1&times; (baseline)</td>
 *     <td>&asymp;1.02–1.05&times; faster</td>
 *   </tr>
 * </table>
 *
 * <h3>Usage Example</h3>
 * @code
 * using jh::serio::huff_algo;
 * using HUF = jh::serio::huffman&lt;"demo", huff_algo::huff256_canonical&gt;;
 *
 * std::ostringstream out_stream(std::ios::binary);
 * HUF::compress(out_stream, "hello world");
 *
 * const std::string binary = out_stream.str();
 *
 * std::istringstream in_stream(binary, std::ios::binary);
 * std::string recovered = HUF::decompress(in_stream);
 * std::cout << recovered << std::endl;
 * @endcode
 *
 * @version <pre>1.4.x</pre>
 * @date <pre>2025</pre>
 */

#pragma once

#include <cstdint>
#include <iostream>
#include <vector>
#include <queue>
#include <stack>
#include <algorithm>
#include "jh/metax/t_str.h"
#include "jh/pods/array.h"
#include "jh/pods/pair.h"

namespace jh::serio {

    namespace detail {

        /// @brief Provides empty canonical_decoder for non-canonical Huffman variants.
        template<bool IsCanonical, size_t N>
        struct canonical_decoder_selector {
            struct type {
            };
        };

        /// @brief Provides canonical_decoder only for canonical Huffman variants.
        template<size_t N>
        struct canonical_decoder_selector<true, N> {
            struct type {
                std::uint8_t code_len[N];      ///< Code length per symbol
                std::uint16_t count[33];       ///< Count of symbols per length L
                std::uint32_t start[33];       ///< Starting code for each length L
                std::uint16_t symbols[33][N];  ///< Symbol lookup table
            };
        };
    }

    /**
     * @brief Enumeration of supported Huffman algorithm variants.
     *
     * @details
     * This enum controls the symbol range (128 or 256)
     * and whether the codec uses standard tree-based Huffman or
     * the deterministic canonical Huffman algorithm.
     */
    enum class huff_algo : std::uint8_t {
        huff128,            ///< Standard Huffman over ASCII (0-127)
        huff256,            ///< Standard Huffman over full byte range (0-255)
        huff128_canonical,  ///< Canonical Huffman for ASCII
        huff256_canonical   ///< Canonical Huffman for full byte range
    };

    /**
     * @brief High-performance Huffman encoder/decoder.
     *
     * <h3>Features</h3>
     * <ul>
     *   <li>ASCII (128 symbols) and full-byte (256 symbols) support</li>
     *   <li>Canonical encoding for deterministic prefix tables</li>
     *   <li>Tree-based Huffman for legacy compatibility</li>
     *   <li>Guaranteed prefix-free encoding with max depth 32</li>
     *   <li>Exception-safe streaming I/O</li>
     * </ul>
     *
     * <h3>Generated file format</h3>
     * <ul>
     *   <li><b>Header</b> — user-defined signature (<code>Signature</code>)</li>
     *   <li><b>Canonical mode</b>: code-length table, total bits, bitstream</li>
     *   <li><b>Standard mode</b>: full frequency table, total bits, bitstream</li>
     * </ul>
     *
     * @tparam Signature  User-defined compile-time signature written to the output
     *                    stream as a format magic number.
     * @tparam Algo       Huffman codec variant (tree-based or canonical).
     */
    template<jh::meta::TStr Signature, huff_algo Algo = huff_algo::huff256_canonical>
    class huffman {

        static constexpr auto signature = Signature;

        static constexpr bool is_canonical =
                Algo == huff_algo::huff128_canonical || Algo == huff_algo::huff256_canonical;

        /// @brief Number of symbols (128 or 256) determined by algorithm type.
        static constexpr size_t table_size =
                ((Algo == huff_algo::huff128 || Algo == huff_algo::huff128_canonical)
                 ? 128 : 256);

        /**
         * @brief Node in the Huffman tree for standard (non-canonical) encoding.
         *
         * @details
         * Internal nodes have <code>left</code> and <code>right</code> children.
         * Leaf nodes contain only <code>ch</code> and <code>freq</code>.
         */
        struct node {
            std::uint16_t ch = 0;   ///< Symbol value (0–127 or 0–255)
            std::uint32_t freq = 0; ///< Symbol frequency
            int left = -1;          ///< Index of left child node
            int right = -1;         ///< Index of right child node
            /// @brief Whether this node is a leaf node.
            [[nodiscard]] bool is_leaf() const { return left < 0 && right < 0; }
        };

        /// @brief Priority queue comparator used when building Huffman trees.
        struct pq_cmp {
            const std::vector<node> *pool;

            explicit pq_cmp(const std::vector<node> *p) : pool(p) {}

            bool operator()(int a, int b) const {
                return (*pool)[a].freq > (*pool)[b].freq;
            }
        };

        /// @brief Huffman code entry: bit pattern + length.
        struct code {
            std::uint32_t bits; ///< Huffman bit pattern (MSB-first)
            std::uint8_t len;   ///< Number of valid bits
        };

        /// @brief Compact fixed-size table of Huffman codes, one per symbol.
        using table_t = jh::pod::array<code, table_size>;

        /**
         * @brief Precomputed tables for O(1) canonical Huffman decoding.
         *
         * @details
         * Contains:
         * <ul>
         *   <li>Length table</li>
         *   <li>Symbol count per length</li>
         *   <li>Start code for each length</li>
         *   <li>Symbol list buckets per length</li>
         * </ul>
         */
        using canonical_decoder =
                typename detail::canonical_decoder_selector<is_canonical, table_size>::type;

        /**
         * @brief Build a Huffman tree from frequency table.
         *
         * @param freq  Symbol frequencies.
         * @param pool  Output vector containing the built nodes.
         * @return Index of the root node (or -1 if empty input).
         */
        static int build_tree(const std::vector<std::uint32_t> &freq,
                              std::vector<node> &pool) {
            pool.clear();
            pool.reserve(table_size * 2);

            for (int i = 0; i < (int) table_size; i++) {
                if (freq[i] > 0) {
                    node n;
                    n.ch = (std::uint16_t) i;
                    n.freq = freq[i];
                    pool.push_back(n);
                }
            }

            if (pool.empty()) return -1;
            if (pool.size() == 1) return 0;

            using queue_t = std::priority_queue<int, std::vector<int>, pq_cmp>;
            queue_t pq{pq_cmp(&pool)};

            for (int i = 0; i < (int) pool.size(); i++)
                pq.push(i);

            while (pq.size() > 1) {
                int a = pq.top();
                pq.pop();
                int b = pq.top();
                pq.pop();

                node p;
                p.left = a;
                p.right = b;
                p.freq = pool[a].freq + pool[b].freq;

                int idx = (int) pool.size();
                pool.push_back(p);
                pq.push(idx);
            }
            return pq.top();
        }

        /**
         * @brief Build raw non-canonical code table by DFS traversal.
         *
         * @param pool  Huffman tree nodes.
         * @param root  Index of tree root.
         * @param tbl   Output prefix code table.
         */
        static void build_code_table(const std::vector<node> &pool,
                                     int root,
                                     table_t &tbl) {
            if (root < 0) return;

            std::stack<jh::pod::pair<int, code>, std::vector<jh::pod::pair<int, code>>> st;
            st.push({root, {0, 0}});

            while (!st.empty()) {
                const auto [i, code] = st.top();
                st.pop();
                const node &n = pool[i];

                if (n.is_leaf()) {
                    tbl[n.ch] = code;
                } else {
                    st.push({
                                    n.left,
                                    {std::uint32_t(code.bits << 1), std::uint8_t(code.len + 1)}
                            });
                    st.push({
                                    n.right,
                                    {std::uint32_t((code.bits << 1) | 1), std::uint8_t(code.len + 1)}
                            });
                }
            }
        }

        /**
         * @brief Build code-length table used for canonical Huffman.
         *
         * @param pool  Huffman nodes.
         * @param root  Root index.
         * @param len_tbl  Output table of code lengths.
         */
        static void build_code_length(const std::vector<node> &pool,
                                      int root,
                                      jh::pod::array<std::uint8_t, table_size> &len_tbl
        ) requires(is_canonical) {
            for (auto &x: len_tbl) x = 0;
            if (root < 0) return;

            std::stack<jh::pod::pair<int, int>, std::vector<jh::pod::pair<int, int>>> st;
            st.push({root, 0});

            while (!st.empty()) {
                const auto [i, depth] = st.top();
                st.pop();
                const node &n = pool[i];

                if (depth > 32)
                    [[unlikely]]
                            throw std::runtime_error("huffman code length exceeds 32 bits");

                if (n.is_leaf()) {
                    len_tbl[n.ch] = depth;
                } else {
                    st.push({n.left, depth + 1});
                    st.push({n.right, depth + 1});
                }
            }
        }

        /**
         * @brief Construct canonical codewords from symbol length table.
         *
         * @param len_tbl Length table.
         * @param tbl     Output code table.
         */
        static void build_canonical_codes(
                const jh::pod::array<std::uint8_t, table_size> &len_tbl,
                table_t &tbl
        ) requires(is_canonical) {
            struct Item {
                int sym;
                int len;
            };
            std::vector<Item> items;
            items.reserve(table_size);

            for (int i = 0; i < (int) table_size; i++)
                if (len_tbl[i] > 0)
                    items.push_back({i, len_tbl[i]});

            if (items.empty()) return;

            std::sort(items.begin(), items.end(),
                      [](auto &a, auto &b) {
                          if (a.len != b.len) return a.len < b.len;
                          return a.sym < b.sym;
                      });

            std::uint32_t code = 0;
            int prev_len = items[0].len;

            for (auto &x: items) {
                if (x.len > prev_len) {
                    code <<= (x.len - prev_len);
                    prev_len = x.len;
                }

                tbl[x.sym].bits = code;
                tbl[x.sym].len = x.len;

                code++;
            }
        }

        /// @brief Build canonical decoder lookup structures.
        static void build_canonical_decoder(
                const jh::pod::array<std::uint8_t, table_size> &len_tbl,
                canonical_decoder &dec
        ) requires(is_canonical){
            dec = {};

            for (int i = 0; i < (int) table_size; i++) {
                std::uint8_t L = len_tbl[i];
                dec.code_len[i] = L;
                if (L > 0)
                    dec.count[L]++;
            }

            std::uint32_t code = 0;
            for (int L = 1; L <= 32; L++) {
                code = (code + dec.count[L - 1]) << 1;
                dec.start[L] = code;
            }

            std::uint16_t offset[33] = {};
            for (int i = 0; i < (int) table_size; i++) {
                std::uint8_t L = dec.code_len[i];
                if (L == 0) continue;
                dec.symbols[L][offset[L]++] = std::uint16_t(i);
            }
        }

        /**
         * @brief Decode bitstream using canonical tables.
         *
         * @param is         Binary input stream positioned at bitstream.
         * @param total_bits Number of bits to read.
         * @param dec        Precomputed canonical tables.
         *
         * @return Decoded plaintext string.
         */
        static std::string canonical_decode(std::istream &is,
                                            std::uint64_t total_bits,
                                            const canonical_decoder &dec
        ) requires(is_canonical) {
            std::string out;
            out.reserve(total_bits / 3);

            std::uint8_t buf = 0;
            int cnt = 0;
            std::uint64_t used = 0;

            std::uint32_t code = 0;
            std::uint32_t L = 0;

            while (used < total_bits) {
                if (cnt == 0) {
                    int b = is.get();
                    if (b == EOF) [[unlikely]]
                        break;
                    buf = std::uint8_t(b);
                    cnt = 8;
                }

                code = (code << 1) | (buf >> 7);
                buf <<= 1;
                cnt--;
                used++;
                L++;

                if (L > 32)
                    [[unlikely]]
                            throw std::runtime_error("huffman code length exceeds 32 bits");

                if (dec.count[L] == 0)
                    continue;

                std::uint32_t start = dec.start[L];
                std::uint32_t end = start + dec.count[L] - 1;

                if (code < start || code > end)
                    continue;

                std::uint32_t idx = code - start;
                out.push_back(char(dec.symbols[L][idx]));

                code = 0;
                L = 0;
            }

            return out;
        }
        
    public:
        
        static void build_code_length(const std::vector<node> &pool,
                                      int root,
                                      jh::pod::array<std::uint8_t, table_size> &len_tbl
        ) requires(!is_canonical) = delete;

        static void build_canonical_codes(
                const jh::pod::array<std::uint8_t, table_size> &len_tbl,
                table_t &tbl
        ) requires(!is_canonical) = delete;

        static void build_canonical_decoder(
                const jh::pod::array<std::uint8_t, table_size> &len_tbl,
                canonical_decoder &dec
        ) requires(!is_canonical) = delete;

        static std::string canonical_decode(std::istream &is,
                                            std::uint64_t total_bits,
                                            const canonical_decoder &dec
        ) requires(!is_canonical) = delete;

        /**
         * @brief Compress a string into the provided binary output stream.
         *
         * <p>
         * Format written depends on <code>Algo</code>:
         * <ul>
         *   <li><b>Canonical</b>: code-lengths → total bits → bitstream</li>
         *   <li><b>Standard</b>: full frequency table → total bits → bitstream</li>
         * </ul>
         * </p>
         *
         * @param os     Output binary stream.
         * @param input  Input string to compress.
         *
         * @throw std::runtime_error  If ASCII-only algorithm receives >127 symbol.
         */
        static void compress(std::ostream &os, std::string_view input) {
            os.write(Signature.val(), Signature.size());

            std::vector<std::uint32_t> freq(table_size);
            for (unsigned char c: input) {
                if constexpr (table_size == 128) {
                    if (c > 127)
                        [[unlikely]]
                                throw std::runtime_error("ASCII only");
                }
                freq[c]++;
            }

            if constexpr (!is_canonical) {
                for (auto f: freq)
                    os.write(reinterpret_cast<const char *>(&f), 4);
            }

            std::vector<node> pool;
            int root = build_tree(freq, pool);

            // ---------- Canonical ----------
            if constexpr (is_canonical) {
                jh::pod::array<std::uint8_t, table_size> len_tbl;
                build_code_length(pool, root, len_tbl);

                for (int i = 0; i < (int) table_size; i++)
                    os.put(len_tbl[i]);

                table_t tbl{};
                build_canonical_codes(len_tbl, tbl);

                std::uint64_t total_bits = 0;
                for (unsigned char c: input)
                    total_bits += tbl[c].len;

                os.write(reinterpret_cast<const char *>(&total_bits), 8);

                std::uint8_t buf = 0;
                int cnt = 0;

                for (unsigned char c: input) {
                    auto cc = tbl[c];
                    for (int i = cc.len - 1; i >= 0; --i) {
                        buf = (buf << 1) | ((cc.bits >> i) & 1);
                        if (++cnt == 8) {
                            os.put(static_cast<char>(buf));
                            buf = 0;
                            cnt = 0;
                        }
                    }
                }
                if (cnt)
                    os.put(static_cast<char>(static_cast<std::uint32_t>(buf << (8 - cnt))));
                return;
            }

            // ---------- Normal huffman 128/256 ----------
            table_t tbl{};
            build_code_table(pool, root, tbl);

            std::uint64_t total_bits = 0;
            for (unsigned char c: input)
                total_bits += tbl[c].len;

            os.write(reinterpret_cast<const char *>(&total_bits), 8);

            std::uint8_t buf = 0;
            int cnt = 0;

            for (unsigned char c: input) {
                auto cc = tbl[c];
                for (int i = cc.len - 1; i >= 0; --i) {
                    buf = (buf << 1) | ((cc.bits >> i) & 1);
                    if (++cnt == 8) {
                        os.put(static_cast<char>(buf));
                        buf = 0;
                        cnt = 0;
                    }
                }
            }
            if (cnt)
                os.put(static_cast<char>(static_cast<std::uint32_t>(buf << (8 - cnt))));
        }

        /**
         * @brief Decompress a Huffman-encoded binary stream.
         *
         * @param is  Input stream positioned at the beginning of the stored format.
         * @return    Decoded textual data.
         *
         * @throw std::runtime_error  On signature mismatch or malformed input.
         */
        static std::string decompress(std::istream &is) {
            char sig[Signature.size()];
            is.read(sig, Signature.size());
            if (std::string_view(sig, Signature.size()) != Signature.view())
                throw std::runtime_error("Bad signature");

            // ---------- Read freq for normal huffman ----------
            std::vector<std::uint32_t> freq(table_size);

            if constexpr (!is_canonical) {
                for (size_t i = 0; i < table_size; i++)
                    is.read(reinterpret_cast<char *>(&freq[i]), 4);
            }

            std::vector<node> pool;
            int root;

            // ---------- Canonical ----------
            if constexpr (is_canonical) {
                jh::pod::array<std::uint8_t, table_size> len_tbl;

                for (int i = 0; i < (int) table_size; i++) {
                    int b = is.get();
                    if (b == EOF)
                        [[unlikely]]
                                throw std::runtime_error("EOF in length table");
                    len_tbl[i] = std::uint8_t(b);
                }

                canonical_decoder dec;
                build_canonical_decoder(len_tbl, dec);

                std::uint64_t total_bits = 0;
                is.read(reinterpret_cast<char *>(&total_bits), 8);

                return canonical_decode(is, total_bits, dec);
            }

            // ---------- Normal huffman Tree ----------
            root = build_tree(freq, pool);

            std::uint64_t total_bits = 0;
            is.read(reinterpret_cast<char *>(&total_bits), 8);

            std::string out;
            out.reserve(total_bits / 3);

            if (root < 0) [[unlikely]]
                return out;

            int node = root;
            std::uint8_t buf = 0;
            int cnt = 0;
            std::uint64_t used = 0;

            while (used < total_bits) {
                if (cnt == 0) {
                    int b = is.get();
                    if (b == EOF) [[unlikely]]
                        break;
                    buf = std::uint8_t(b);
                    cnt = 8;
                }

                int bit = (buf >> 7) & 1;
                buf <<= 1;
                cnt--;
                used++;

                node = bit ? pool[node].right : pool[node].left;

                if (pool[node].is_leaf()) {
                    out.push_back(char(pool[node].ch));
                    node = root;
                }
            }

            return out;
        }
    };
} // namespace jh::serio
