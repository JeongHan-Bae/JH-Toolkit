#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "jh/pattern_matcher"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

namespace test {
    using namespace jh;
    using namespace jh::pattern;

    struct number_rule {
        [[maybe_unused]] static constexpr std::size_t rank = 10;

        static constexpr std::pair<bool, std::size_t> match(std::string_view input) {
            std::size_t length = 0;
            while (length < input.size() && input[length] >= '0' && input[length] <= '9') {
                ++length;
            }
            return {length > 0, length};
        }
    };

    struct segment_rule {
        [[maybe_unused]] static constexpr std::size_t rank = 20;

        static constexpr std::pair<bool, std::size_t> match(std::string_view input) {
            const auto slash = input.find('/');
            const auto length = slash == std::string_view::npos ? input.size() : slash;
            return {length > 0, length};
        }
    };

    using builder = pattern_matcher_builder<int, 512, number_rule{}, segment_rule{}>;

    void routes_and_captures() {
        builder routes;
        routes.insert({{{"/api", none{}}}, 1});
        routes.insert({{{"/api/", rule_tag<number_rule{}>{}},
                        {"/info", none{}}}, 2});
        routes.insert({{{"/api/", rule_tag<segment_rule{}>{}},
                        {"/info", none{}}}, 3});
        routes.insert({{{"/api/", rule_tag<number_rule{}>{}},
                        {"/posts/", rule_tag<number_rule{}>{}},
                        {"/a", none{}}}, 4});
        routes.insert({{{"/api/", all{}}}, 5});
        tiny_test::expect(routes.size() == 5, "five routes registered");

        auto tree = std::move(routes).build();
        auto exact = tree.match("/api");
        tiny_test::expect(exact && exact.value().second == 1 &&
                          exact.value().first.empty(),
                          "exact route succeeds with zero captures");
        auto numeric = tree.match("/api/123/info");
        tiny_test::expect(numeric && numeric.value().second == 2 &&
                          numeric.value().first.size() == 1 && numeric.value().first[0] == "123",
                          "lower-rank numeric rule wins");
        auto segment = tree.match("/api/abc/info");
        tiny_test::expect(segment && segment.value().second == 3 &&
                          segment.value().first.size() == 1 && segment.value().first[0] == "abc",
                          "segment rule matches nonnumeric input");
        auto nested = tree.match("/api/12/posts/34/a");
        tiny_test::expect(nested && nested.value().second == 4 &&
                          nested.value().first.size() == 2 &&
                          nested.value().first[0] == "12" && nested.value().first[1] == "34",
                          "nested rules capture in route order");
        auto fallback = tree.match("/api/blah/blah");
        tiny_test::expect(fallback && fallback.value().second == 5 &&
                          fallback.value().first.size() == 1 &&
                          fallback.value().first[0] == "blah/blah",
                          "catch-all captures the suffix");
        auto missing = tree.match("/xyz");
        tiny_test::expect(!missing && missing.error() == match_error::unmatched,
                          "unmatched path reports an error");
        tiny_test::expect(tree.pool_size() > 0 && tree.index_node_count() > 0,
                          "built tree exposes storage counts");
    }

    template<std::size_t Limit>
    auto ambiguous_routes() {
        pattern_matcher_builder<int, Limit, number_rule{}, segment_rule{}> routes;
        routes.insert({{{"/a/", rule_tag<number_rule{}>{}},
                        {"/x", none{}}}, 1});
        routes.insert({{{"/a/", rule_tag<segment_rule{}>{}},
                        {"/y", none{}}}, 2});
        return std::move(routes).build();
    }

    void backtracking() {
        auto limited = ambiguous_routes<0>();
        auto over = limited.match("/a/123/y");
        tiny_test::expect(!over && over.error() == match_error::backtrack_limit_exceeded,
                          "zero limit rejects a second viable branch");
        auto missing = limited.match("/nope");
        tiny_test::expect(!missing && missing.error() == match_error::unmatched,
                          "missing path does not consume the limit");

        auto allowed = ambiguous_routes<1>();
        auto matched = allowed.match("/a/123/y");
        tiny_test::expect(matched && matched.value().second == 2 &&
                          matched.value().first.size() == 1 && matched.value().first[0] == "123",
                          "one backtrack reaches the second rule");
    }

    struct nonmovable {
        explicit nonmovable(int number) : value(number) {}
        nonmovable(const nonmovable&) = delete;
        nonmovable(nonmovable&&) = delete;
        int value;
    };

    void reference_query() {
        pattern_matcher_builder<std::unique_ptr<nonmovable>> routes;
        routes.insert({{{"", none{}}}, std::make_unique<nonmovable>(42)});
        auto tree = std::move(routes).build();
        auto found = tree.match_ref("");
        tiny_test::expect(found && found.value().second->get()->value == 42,
                          "reference query supports move-only values");

        struct nondefault_copy {
            explicit nondefault_copy(int number) : value(number) {}
            int value;
        };
        pattern_matcher_builder<nondefault_copy> nondefault_routes;
        nondefault_routes.insert({{{"value", none{}}}, nondefault_copy{9}});
        auto nondefault_matcher = std::move(nondefault_routes).build();
        auto reference = nondefault_matcher.match_ref("value");
        tiny_test::expect(reference && reference.value().second->value == 9,
                          "reference query does not require default construction of T");
    }

    void literal_and_empty_trees() {
        pattern_matcher_builder<int> routes;
        routes.insert({{{"/one", none{}}}, 1});
        routes.insert({{{"/two", none{}}}, 2});
        auto tree = std::move(routes).build();
        tiny_test::expect(tree.match("/one").value().second == 1, "first literal route");
        tiny_test::expect(tree.match("/two").value().second == 2, "second literal route");
        tiny_test::expect(!tree.match("/on"), "partial literal is not a match");

        pattern_matcher_builder<int> empty_routes;
        auto empty_tree = std::move(empty_routes).build();
        tiny_test::expect(!empty_tree.match("/anything"), "empty tree has no matches");
    }

    void invalid_routes() {
        tiny_test::expect_throw<std::invalid_argument>([] {
            pattern_matcher_builder<int> routes;
            routes.insert({{}, 1});
            auto tree = std::move(routes).build();
            (void)tree;
        });
        tiny_test::expect_throw<std::invalid_argument>([] {
            pattern_matcher_builder<int> routes;
            routes.insert({{{"/same", none{}}}, 1});
            routes.insert({{{"/same", none{}}}, 2});
            auto tree = std::move(routes).build();
            (void)tree;
        });
        tiny_test::expect_throw<std::invalid_argument>([] {
            pattern_matcher_builder<int> routes;
            routes.insert({{{"a", none{}}, {"b", none{}}}, 1});
            auto tree = std::move(routes).build();
            (void)tree;
        });
        tiny_test::expect_throw<std::invalid_argument>([] {
            pattern_matcher_builder<int> routes;
            routes.insert({{{"a", all{}}, {"b", none{}}}, 1});
            auto tree = std::move(routes).build();
            (void)tree;
        });
        tiny_test::expect_throw<std::invalid_argument>([] {
            pattern_matcher_builder<int> routes;
            routes.insert({{{"a", all{}}}, 1});
            routes.insert({{{"a", all{}}}, 2});
            auto tree = std::move(routes).build();
            (void)tree;
        });
    }

    void precedence() {
        pattern_matcher_builder<int, 512, segment_rule{}> routes;
        routes.insert({{{"abc", none{}}}, 1});
        routes.insert({{{"", rule_tag<segment_rule{}>{}}}, 2});
        routes.insert({{{"", all{}}}, 3});
        auto matcher = std::move(routes).build();
        tiny_test::expect(matcher.match("abc").value().second == 1,
                          "complete literal precedes custom and catch-all");
        tiny_test::expect(matcher.match("other").value().second == 2,
                          "custom rule precedes catch-all");
        tiny_test::expect(matcher.match("a/b").value().second == 3,
                          "catch-all handles input rejected by complete custom rule");
    }

    void empty_and_moved_results() {
        pattern_matcher_builder<int> routes;
        routes.insert({{{"", none{}}}, 1});
        routes.insert({{{"x", all{}}}, 2});
        auto matcher = std::move(routes).build();

        auto exact = matcher.match("");
        tiny_test::expect(exact && exact.value().first.empty() &&
                          exact.value().first.data() == nullptr,
                          "empty input can match with zero captures");
        auto empty_capture = matcher.match("x");
        tiny_test::expect(empty_capture && empty_capture.value().first.size() == 1 &&
                          empty_capture.value().first[0].empty(),
                          "catch-all can capture an empty suffix");
        auto moved = std::move(empty_capture);
        tiny_test::expect(moved && moved.value().first.size() == 1 &&
                          moved.value().second == 2,
                          "moving a match result preserves its captures and value");
        moved.value().first[0] = "replacement";
        tiny_test::expect(moved.value().first[0] == "replacement",
                          "fixed-length capture elements remain assignable");
    }

    void input_views_and_repeated_queries() {
        pattern_matcher_builder<int, 512, segment_rule{}> routes;
        routes.insert({{{"/pre/", rule_tag<segment_rule{}>{}},
                        {"/post", none{}}}, 7});
        auto matcher = std::move(routes).build();
        std::string input = "/pre/middle/post";
        auto first = matcher.match(input);
        tiny_test::expect(first && first.value().first.size() == 1 &&
                          first.value().first[0] == "middle" &&
                          first.value().first[0].data() == input.data() + 5,
                          "capture view points into the original input");
        auto second = matcher.match("/pre/other/post");
        tiny_test::expect(second && second.value().first[0] == "other" &&
                          first.value().first[0] == "middle",
                          "read-only queries retain independent capture results");
        std::string owned(first.value().first[0]);
        input.clear();
        tiny_test::expect(owned == "middle", "capture can be copied before input changes");
    }

    void moved_matchers() {
        pattern_matcher_builder<int> routes;
        routes.insert({{{"key", none{}}}, 3});
        auto original = std::move(routes).build();
        pattern_matcher<int> moved(std::move(original));
        tiny_test::expect(moved.match("key").value().second == 3,
                          "move construction preserves matching");
        auto referenced = moved.match_ref("key");
        tiny_test::expect(referenced && *referenced.value().second == 3,
                          "reference query reaches the moved matcher's value");

        pattern_matcher_builder<int> other_routes;
        other_routes.insert({{{"other", none{}}}, 4});
        auto assigned = std::move(other_routes).build(); // NOLINT for test
        assigned = std::move(moved);
        tiny_test::expect(assigned.match("key").value().second == 3 &&
                          !assigned.match("other"),
                          "move assignment replaces the matcher index");
    }
}

template<>
struct jh::test::tiny_test::test<"routes and captures">
    : jh::test::tiny_test::test_definition<"routes and captures", &::test::routes_and_captures> {};
template<>
struct jh::test::tiny_test::test<"backtracking">
    : jh::test::tiny_test::test_definition<"backtracking", &::test::backtracking> {};
template<>
struct jh::test::tiny_test::test<"reference query">
    : jh::test::tiny_test::test_definition<"reference query", &::test::reference_query> {};
template<>
struct jh::test::tiny_test::test<"literal and empty trees">
    : jh::test::tiny_test::test_definition<"literal and empty trees", &::test::literal_and_empty_trees> {};
template<>
struct jh::test::tiny_test::test<"invalid routes">
    : jh::test::tiny_test::test_definition<"invalid routes", &::test::invalid_routes> {};
template<>
struct jh::test::tiny_test::test<"precedence">
    : jh::test::tiny_test::test_definition<"precedence", &::test::precedence> {};
template<>
struct jh::test::tiny_test::test<"empty and moved results">
    : jh::test::tiny_test::test_definition<"empty and moved results", &::test::empty_and_moved_results> {};
template<>
struct jh::test::tiny_test::test<"input views and repeated queries">
    : jh::test::tiny_test::test_definition<"input views and repeated queries", &::test::input_views_and_repeated_queries> {};
template<>
struct jh::test::tiny_test::test<"moved matchers">
    : jh::test::tiny_test::test_definition<"moved matchers", &::test::moved_matchers> {};
template<>
struct jh::test::tiny_test::session<"pattern matcher">
    : jh::test::tiny_test::session_definition<
          "pattern matcher",
          jh::test::tiny_test::test<"routes and captures">,
          jh::test::tiny_test::test<"backtracking">,
          jh::test::tiny_test::test<"reference query">,
          jh::test::tiny_test::test<"literal and empty trees">,
          jh::test::tiny_test::test<"invalid routes">,
          jh::test::tiny_test::test<"precedence">,
          jh::test::tiny_test::test<"empty and moved results">,
          jh::test::tiny_test::test<"input views and repeated queries">,
          jh::test::tiny_test::test<"moved matchers">> {};

namespace test {
    [[maybe_unused]] const tiny_test::session<"pattern matcher"> registered_session{};
}
