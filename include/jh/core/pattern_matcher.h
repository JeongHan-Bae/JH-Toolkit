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
 * @file pattern_matcher.h
 * @brief Runtime pattern matcher backed by an immutable compressed radix index.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 */

#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "jh/metax/expected.h"
#include "jh/runtime_arr"

namespace jh::pattern {

    /// @brief Ends a pattern after its literal portion; produces no capture.
    struct none final {
    };
    /// @brief Captures all remaining input, including an empty suffix, and ends a pattern.
    struct all final {
    };

    /**
     * @brief Identifies a custom rule supplied as a template argument to the matcher.
     * @tparam Rule Rule object used for matching part of an input.
     */
    template<auto Rule>
    struct rule_tag final {
        static constexpr auto value = Rule;
    };

    /**
     * @brief Checks that a rule exposes a constant rank and reports its matched length.
     *
     * A lower <code>rank</code> is tried before a higher one.
     * A successful rule returns <code>{true, consumed}</code>, where
     * <code>consumed</code> is no greater than the supplied input length.
     * The matcher captures precisely that prefix and may try another rule if
     * the rest of the pattern fails.
     * @tparam Rule Custom pattern rule type.
     */
    template<class Rule>
    concept rule = requires(const Rule &rule, std::string_view input) {
        typename std::integral_constant<std::size_t,
                std::remove_cvref_t<Rule>::rank>;
        { rule.match(input) } -> std::same_as<std::pair<bool, std::size_t>>;
    };

    /// @brief Describes why a complete pattern match could not be produced.
    enum class match_error : std::uint8_t {
        unmatched,
        backtrack_limit_exceeded
    };

    /**
     * @brief Match result containing a fixed-length capture array and a value, or an error.
     *
     * The <code>jh::runtime_arr&lt;std::string_view&gt;</code> owns its element
     * buffer and is move-only. Its length is chosen at runtime by the successful
     * pattern. Elements remain mutable, but the array cannot grow or shrink.
     * Conversion from the search vector moves elements into a separate allocation;
     * it does not take over the vector's buffer.
     *
     * @warning Every capture is a non-owning slice of the supplied input. Moving
     * the array or result does not extend the input's lifetime. The input's
     * characters must remain alive, accessible, and unmodified in a way that
     * invalidates views. To persist a capture after input destruction, copy it
     * while input is still valid:
     * @code
     * std::string owned_capture(result.value().first[0]);
     * @endcode
     * The returned array can outlive the input, but its views may then dangle.
     *
     * @tparam T Returned value or pointer type. The underlying expected requires
     * its success value to be default constructible for error results.
     */
    template<class T>
    using match_result = jh::meta::expected<
            std::pair<jh::runtime_arr<std::string_view>, T>, match_error>;

} // namespace jh::pattern

namespace jh::radix {

    using jh::pattern::all;
    using jh::pattern::none;
    using jh::pattern::rule_tag;
    using jh::pattern::rule;
    using jh::pattern::match_error;
    using jh::pattern::match_result;

    namespace detail {

        template<auto... Rules>
        concept unique_rule_ranks = [] {
            constexpr std::array<std::size_t, sizeof...(Rules)> ranks{
                    std::remove_cvref_t<decltype(Rules)>::rank...
            };
            for (std::size_t i = 0; i < ranks.size(); ++i) {
                for (std::size_t j = i + 1; j < ranks.size(); ++j) {
                    if (ranks[i] == ranks[j]) return false;
                }
            }
            return true;
        }();

        template<class T, auto... Rules>
        requires (std::movable<T> && (rule<decltype(Rules)> && ...) &&
                  unique_rule_ranks<Rules...>)
        struct model {
            using rule_variant = std::variant<
                    none, all, rule_tag < Rules>...>;
            using pattern_type = std::vector<std::pair<std::string, rule_variant>>;

            struct entry {
                pattern_type pattern;
                T value;
            };

            static std::size_t rank(const rule_variant &rule) {
                return std::visit([](const auto &tag) -> std::size_t {
                    using Tag = std::remove_cvref_t<decltype(tag)>;
                    if constexpr (std::same_as<Tag, none>) {
                        return 0;
                    } else if constexpr (std::same_as<Tag, all>) {
                        return (std::numeric_limits<std::size_t>::max)();
                    } else {
                        return std::remove_cvref_t<decltype(Tag::value)>::rank;
                    }
                }, rule);
            }

            static std::pair<bool, std::size_t>
            match(const auto &rule, [[maybe_unused]] std::string_view input) {
                return std::visit([&](const auto &tag)
                                          -> std::pair<bool, std::size_t> {
                    using Tag = std::remove_cvref_t<decltype(tag)>;
                    if constexpr (std::same_as<Tag, none>) {
                        return {input.empty(), 0};
                    } else if constexpr (std::same_as<Tag, all>) {
                        return {true, input.size()};
                    } else if constexpr (std::same_as<Tag, std::string_view>) {
                        return {false, 0};
                    } else {
                        return Tag::value.match(input);
                    }
                }, rule);
            }
        };

    } // namespace detail

    /**
     * @brief Collects patterns before producing an immutable radix tree.
     * @tparam T Stored value type; must be movable.
     * @tparam BacktrackLimit Maximum number of failed-branch switches per match.
     * @tparam Rules Custom rule objects; their <code>rank</code> values must be distinct.
     */
    template<class T, std::size_t BacktrackLimit = 512, auto... Rules>
    class radix_tree_builder;

    /**
     * @brief Immutable matcher for literal patterns and custom rule segments.
     *
     * Literal branches take precedence over custom rules. Custom rules are tried
     * in ascending <code>rank</code> order, and a catch-all pattern is tried last.
     * Matching requires the complete input. The matcher can be queried repeatedly
     * after construction; its pattern set is immutable. A backtrack is counted
     * only when switching from a failed descended branch to another viable branch.
     *
     * @warning All captures are non-owning <code>std::string_view</code> slices
     * into the supplied input. The returned <code>runtime_arr</code> owns only its
     * view elements, not the input characters. Moving the result does not extend
     * the input lifetime. Keep the input alive, accessible, and unchanged in any
     * way that invalidates views. Copy captures to <code>std::string</code> before
     * the input expires when they must persist.
     *
     * @tparam T Stored value type; must be movable.
     * @tparam BacktrackLimit Maximum number of failed-branch switches per match.
     * @tparam Rules Custom rule objects with distinct <code>rank</code> values.
     */
    template<class T, std::size_t BacktrackLimit = 512, auto... Rules>
    class frozen_radix_tree final {
    public:
        /// @brief Internal pattern model used by the matcher and its builder.
        using model_type = detail::model<T, Rules...>;
        /// @brief Variant of terminal, catch-all, and custom matching rules.
        using rule_variant = typename model_type::rule_variant;
        /// @brief Ordered literal and rule pairs defining one pattern.
        using pattern_type = typename model_type::pattern_type;
        /// @brief Pattern paired with a stored value.
        using entry = typename model_type::entry;

        /// @brief Configured maximum number of failed-branch switches.
        static constexpr std::size_t backtrack_limit = BacktrackLimit;

    private:
        using node_id = std::size_t;
        using head_type = std::variant<
                std::string_view, none, all,
                rule_tag<Rules>...>;

        struct child_edge {
            head_type head;
            node_id child;
        };

        struct index_node {
            std::string_view prefix;
            std::vector<child_edge> children;
            std::optional<std::size_t> value_index; // terminal leaf only
        };

        struct temporary_node {
            std::vector<std::pair<unsigned char, node_id>> literal;
            std::vector<std::pair<rule_variant, node_id>> func;
            std::optional<std::size_t> terminal;
            std::optional<std::size_t> all;
        };

        struct slice {
            std::size_t begin, length;
        };

        enum class search_state : std::uint8_t {
            matched, failed, limit
        };
        struct context {
            std::vector<std::string_view> captures;
            std::size_t value_index = 0;
            std::size_t backtracks = 0;
        };

        // The pool must never be mutated after binding views.
        std::unique_ptr<char[]> pool_;
        std::size_t pool_size_ = 0;
        std::vector<index_node> index_;
        std::vector<T> values_;

        // A null-terminated extra byte is allocated for convenience, but is not
        // included in any string_view. No trailing-zero requirement on patterns.
        static std::size_t add_temporary_node(std::vector<temporary_node> &nodes) {
            const auto id = nodes.size();
            nodes.emplace_back();
            return id;
        }

        static node_id literal_child(
                std::vector<temporary_node> &nodes, node_id source,
                unsigned char ch) {
            for (const auto &[first, child]: nodes[source].literal) {
                if (first == ch) return child;
            }
            const auto next = add_temporary_node(nodes);
            nodes[source].literal.emplace_back(ch, next);
            return next;
        }

        static node_id func_child(
                std::vector<temporary_node> &nodes, node_id source,
                const rule_variant &rule) {
            for (const auto &[stored, child]: nodes[source].func) {
                if (stored.index() == rule.index()) return child;
            }
            const auto next = add_temporary_node(nodes);
            nodes[source].func.emplace_back(rule, next);
            return next;
        }

        static void insert_temporary(
                std::vector<temporary_node> &nodes,
                const pattern_type &pattern, std::size_t value_index) {
            if (pattern.empty()) throw std::invalid_argument("empty pattern");
            node_id at = 0;
            for (std::size_t j = 0; j < pattern.size(); ++j) {
                const auto &[literal, rule] = pattern[j];
                for (unsigned char ch: literal) {
                    at = literal_child(nodes, at, ch);
                }
                const bool last = j + 1 == pattern.size();
                if (std::holds_alternative<none>(rule)) {
                    if (!last) throw std::invalid_argument("none rule must be last");
                    if (nodes[at].terminal) throw std::invalid_argument("duplicate pattern");
                    nodes[at].terminal = value_index;
                } else if (std::holds_alternative<all>(rule)) {
                    if (!last) throw std::invalid_argument("all rule must be last");
                    if (nodes[at].all) throw std::invalid_argument("duplicate catch-all pattern");
                    nodes[at].all = value_index;
                } else {
                    at = func_child(nodes, at, rule);
                    if (last) {
                        // The end of the pattern implies a final `none`.
                        if (nodes[at].terminal) throw std::invalid_argument("duplicate pattern");
                        nodes[at].terminal = value_index;
                    }
                }
            }
        }

        // Allocate a leaf for a value. The parent edge is tagged `none` or `all`.
        node_id create_leaf(std::size_t value_index,
                            std::vector<slice> &offsets,
                            std::size_t current_pool_size) {
            const node_id id = index_.size();
            index_.push_back(index_node{{}, {}, value_index});
            offsets.push_back({current_pool_size, 0});
            return id;
        }

        node_id compress_temporary(
                const std::vector<temporary_node> &tmp,
                node_id current,
                std::string prefix,
                std::vector<char> &buffer,
                std::vector<slice> &offsets) {

            // A chain of exactly one literal child can be compressed as long as
            // it has neither a terminal nor any rule/alternative branch.
            while (!tmp[current].terminal && !tmp[current].all &&
                   tmp[current].func.empty() && tmp[current].literal.size() == 1) {
                auto [first, next] = tmp[current].literal.front();
                prefix.push_back(static_cast<char>(first));
                current = next;
            }

            const node_id id = index_.size();
            index_.push_back(index_node{});
            offsets.push_back({buffer.size(), prefix.size()});
            buffer.insert(buffer.end(), prefix.begin(), prefix.end());

            if (tmp[current].terminal) {
                const auto leaf = create_leaf(*tmp[current].terminal, offsets, buffer.size());
                index_[id].children.push_back({none{}, leaf});
            }

            // Exactly one literal child per initial byte, sorted for lower_bound.
            auto literals = tmp[current].literal;
            std::sort(literals.begin(), literals.end(),
                      [](const auto &a, const auto &b) { return a.first < b.first; });
            for (const auto &[first, next]: literals) {
                const auto child = compress_temporary(
                        tmp, next, std::string(1, static_cast<char>(first)),
                        buffer, offsets);
                // The actual view is set after pool_ has been allocated.
                index_[id].children.push_back({std::string_view{}, child});
            }

            auto funcs = tmp[current].func;
            std::sort(funcs.begin(), funcs.end(), [](const auto &a, const auto &b) {
                return model_type::rank(a.first) < model_type::rank(b.first);
            });
            for (const auto &[rule, next]: funcs) {
                const auto child = compress_temporary(tmp, next, {}, buffer, offsets);
                // Converts rule_variant to head_type (string_view is a new alt).
                std::visit([&](const auto &typed_rule) {
                    index_[id].children.push_back({head_type{typed_rule}, child});
                }, rule);
            }

            if (tmp[current].all) {
                const auto leaf = create_leaf(*tmp[current].all, offsets, buffer.size());
                index_[id].children.push_back({all{}, leaf});
            }
            return id;
        }

        void build(std::vector<entry> &&entries) {
            std::vector<temporary_node> temp(1);
            for (std::size_t i = 0; i < entries.size(); ++i) {
                insert_temporary(temp, entries[i].pattern, i);
            }

            std::vector<char> buffer;
            std::vector<slice> offsets;
            compress_temporary(temp, 0, {}, buffer, offsets);

            // pool_ is created exactly once and never reallocated.
            pool_size_ = buffer.size();
            pool_ = std::make_unique<char[]>(pool_size_ + 1);
            if (pool_size_ > 0)
                std::memcpy(pool_.get(), buffer.data(), pool_size_);
            pool_[pool_size_] = '\0';

            for (std::size_t i = 0; i < index_.size(); ++i) {
                index_[i].prefix = {
                        pool_.get() + offsets[i].begin,
                        offsets[i].length
                };
            }
            for (auto &node: index_) {
                for (auto &edge: node.children) {
                    if (auto *literal = std::get_if<std::string_view>(&edge.head)) {
                        *literal = index_[edge.child].prefix;
                    }
                }
            }

            values_.reserve(entries.size());
            for (auto &e: entries) values_.push_back(std::move(e.value));
        }

        // Return the first matching complete input. A "backtrack" is charged only
        // when switching from one failed descended branch to another viable one.
        // Exhausting a single branch without alternatives costs zero backtracks.
        search_state search(node_id id, std::string_view remaining,
                            context &ctx) const {
            const auto &node = index_[id];
            if (!remaining.starts_with(node.prefix)) return search_state::failed;
            remaining.remove_prefix(node.prefix.size());

            if (node.value_index) {
                if (!remaining.empty()) return search_state::failed;
                ctx.value_index = *node.value_index;
                return search_state::matched;
            }

            bool attempted_branch = false;
            auto descend = [&](node_id child, std::string_view rest,
                               std::optional<std::string_view> capture) -> search_state {
                if (attempted_branch && ++ctx.backtracks > backtrack_limit)
                    return search_state::limit;
                attempted_branch = true;
                const auto saved = ctx.captures.size();
                if (capture) ctx.captures.push_back(*capture);
                const auto result = search(child, rest, ctx);
                if (result == search_state::matched) return result;
                ctx.captures.resize(saved);
                return result;
            };

            // Node invariant: none, sorted exact, sorted funcs, all.
            auto begin = node.children.begin();
            auto end = node.children.end();
            if (remaining.empty() && begin != end &&
                std::holds_alternative<none>(begin->head)) {
                auto result = descend(begin->child, remaining, std::nullopt);
                if (result != search_state::failed) return result;
            }

            auto exact_begin = begin;
            if (exact_begin != end &&
                std::holds_alternative<none>(exact_begin->head))
                ++exact_begin;
            auto exact_end = exact_begin;
            while (exact_end != end &&
                   std::holds_alternative<std::string_view>(exact_end->head))
                ++exact_end;

            if (!remaining.empty()) {
                const auto needle = static_cast<unsigned char>(remaining.front());
                auto it = std::lower_bound(exact_begin, exact_end, needle,
                                           [](const child_edge &edge, unsigned char c) {
                                               return static_cast<unsigned char>(
                                                              std::get<std::string_view>(edge.head).front()) < c;
                                           });
                if (it != exact_end &&
                    static_cast<unsigned char>(
                            std::get<std::string_view>(it->head).front()) == needle) {
                    const auto result = descend(it->child, remaining, std::nullopt);
                    if (result != search_state::failed) return result;
                }
            }

            for (auto it = exact_end; it != end; ++it) {
                if (std::holds_alternative<all>(it->head)) {
                    const auto result = descend(it->child, {}, remaining);
                    if (result != search_state::failed) return result;
                    continue;
                }
                auto [ok, consumed] = model_type::match(it->head, remaining);
                if (!ok || consumed > remaining.size()) continue;
                const auto result = descend(it->child, remaining.substr(consumed),
                                            remaining.substr(0, consumed));
                if (result != search_state::failed) return result;
            }
            return search_state::failed;
        }

        // Use named private ctor, so only builder can construct from patterns.
        struct build_tag {
        };

        explicit frozen_radix_tree(build_tag, std::vector<entry> &&entries) {
            build(std::move(entries));
        }

        friend class radix_tree_builder<T, BacktrackLimit, Rules...>;

    public:
        frozen_radix_tree(const frozen_radix_tree &) = delete;

        frozen_radix_tree &operator=(const frozen_radix_tree &) = delete;

        frozen_radix_tree(frozen_radix_tree &&) noexcept = default;

        frozen_radix_tree &operator=(frozen_radix_tree &&) noexcept = default;

        /// @brief Returns the number of literal bytes stored in the tree's pool.
        [[nodiscard]] std::size_t pool_size() const noexcept { return pool_size_; }

        /// @brief Returns the number of index nodes, including terminal leaves.
        [[nodiscard]] std::size_t index_node_count() const noexcept { return index_.size(); }

        /**
         * @brief Matches the complete input and copies its value into the result.
         * @param input Text to match; its characters must outlive returned captures.
         * @return Captures and value, or an unmatched or backtrack-limit error.
         * @warning Captures are non-owning slices of <code>input</code>. Moving the
         * result does not preserve input storage. Copy captures to owning strings
         * while input is valid if they must outlive it.
         * This overload exists only when <code>T</code> is copy constructible and
         * default initializable, as required by the error path of
         * <code>jh::meta::expected</code>.
         */
        [[nodiscard]] match_result<T>
        match(std::string_view input) const requires (std::copy_constructible<T> && std::default_initializable<T>) {
            context ctx;
            const auto state = search(0, input, ctx);
            if (state == search_state::limit)
                return jh::meta::unexpected{match_error::backtrack_limit_exceeded};
            if (state == search_state::failed)
                return jh::meta::unexpected{match_error::unmatched};
            return std::pair<jh::runtime_arr<std::string_view>, T>{
                    jh::runtime_arr<std::string_view>(std::move(ctx.captures)), values_[ctx.value_index]
            };
        }

        /**
         * @brief Matches the complete input and returns a pointer to its stored value.
         * @param input Text to match; its characters must outlive returned captures.
         * @return Captures and a value pointer, or an unmatched or backtrack-limit error.
         * @warning Captures are non-owning slices of <code>input</code>. Moving the
         * result does not preserve input storage. Copy captures to owning strings
         * while input is valid if they must outlive it.
         * The value pointer does not copy or transfer the value. It remains valid
         * while the matcher and its values are alive; moving the matcher transfers
         * its storage, so use the moved-to matcher to establish new pointers.
         * Destruction or move assignment of the owning matcher invalidates it.
         */
        [[nodiscard]] match_result<const T *> match_ref(std::string_view input) const {
            context ctx;
            const auto state = search(0, input, ctx);
            if (state == search_state::limit)
                return jh::meta::unexpected{match_error::backtrack_limit_exceeded};
            if (state == search_state::failed)
                return jh::meta::unexpected{match_error::unmatched};
            return std::pair<jh::runtime_arr<std::string_view>, const T *>{
                    jh::runtime_arr<std::string_view>(std::move(ctx.captures)),
                    std::addressof(values_[ctx.value_index])
            };
        }
    };

    template<class T, std::size_t BacktrackLimit, auto... Rules>
    class radix_tree_builder final {
    public:
        /// @brief Matcher type produced by this builder.
        using tree_type = frozen_radix_tree<T, BacktrackLimit, Rules...>;
        /// @brief Variant of terminal, catch-all, and custom matching rules.
        using rule_variant [[maybe_unused]] = typename tree_type::rule_variant;
        /// @brief Ordered literal and rule pairs defining one pattern.
        using pattern_type [[maybe_unused]] = typename tree_type::pattern_type;
        /// @brief Pattern paired with a stored value.
        using entry = typename tree_type::entry;

    private:
        std::vector<entry> entries_;

    public:
        /// @brief Creates an empty pattern builder.
        radix_tree_builder() = default;

        radix_tree_builder(const radix_tree_builder &) = delete;

        radix_tree_builder &operator=(const radix_tree_builder &) = delete;

        radix_tree_builder(radix_tree_builder &&) = default;

        radix_tree_builder &operator=(radix_tree_builder &&) = default;

        /// @brief Appends a pattern with a movable value; registration is not thread-safe.
        void insert(entry pattern) { entries_.push_back(std::move(pattern)); }

        /// @brief Reserves capacity for pattern entries.
        void reserve(std::size_t n) { entries_.reserve(n); }

        /// @brief Returns the number of registered patterns.
        [[nodiscard]] std::size_t size() const noexcept { return entries_.size(); }

        /**
         * @brief Consumes the builder and produces an immutable tree.
         * @return Immutable matcher containing all registered patterns.
         * @throws std::invalid_argument If a matcher is empty, a terminal rule is
         * followed by another segment, or a pattern is duplicated.
         */
        [[nodiscard]] tree_type build() &&{
            return tree_type{typename tree_type::build_tag{}, std::move(entries_)};
        }
    };

} // namespace jh::radix

namespace jh {

    /**
     * @brief Immutable runtime pattern matcher backed by a compressed radix index.
     *
     * Matching consumes the complete input. Literal choices precede custom rules
     * in ascending <code>rank</code> order; catch-all is last. Successful captures
     * are stored in a move-only, fixed-length <code>jh::runtime_arr</code>.
     *
     * @warning Captures are non-owning <code>std::string_view</code> slices of the
     * original input. The array owns only its view elements. Keep the input alive,
     * accessible, and free of invalidating modification while reading captures.
     * Moving a result does not own or preserve input characters. Copy any capture
     * needed beyond the input lifetime to <code>std::string</code> first.
     * @tparam T Movable value stored for each pattern.
     * @tparam BacktrackLimit Maximum failed-branch switches per match.
     * @tparam Rules Custom rule objects with distinct compile-time ranks.
     */
    template<class T, std::size_t BacktrackLimit = 512, auto... Rules>
    using pattern_matcher = radix::frozen_radix_tree<T, BacktrackLimit, Rules...>;

    /**
     * @brief Collects patterns and produces an immutable pattern matcher.
     * @tparam T Movable value stored for each pattern.
     * @tparam BacktrackLimit Maximum failed-branch switches per match.
     * @tparam Rules Custom rule objects with distinct compile-time ranks.
     */
    template<class T, std::size_t BacktrackLimit = 512, auto... Rules>
    using pattern_matcher_builder = radix::radix_tree_builder<T, BacktrackLimit, Rules...>;

} // namespace jh
