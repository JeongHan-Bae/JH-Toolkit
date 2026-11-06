#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

#include <mutex>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

#include "jh/immutable_str"
#include "../support/pool_test_mutex.hpp"

namespace test {
    using jh::test::tiny_test::expect;

    template<bool AutoTrim>
    void check_policy() {
        using str_type = jh::immutable_str<AutoTrim>;
        using shared_type = jh::shared_str_ptr<AutoTrim>;
        using weak_type = jh::weak_str_ptr<AutoTrim>;
        using hash_type = jh::atomic_str_hash<AutoTrim>;
        using eq_type = jh::atomic_str_eq<AutoTrim>;

        const std::string expected = AutoTrim ? "hello" : " \t hello \r ";
        str_type value{" \t hello \r "};
        str_type null_value{nullptr};
        expect(null_value.view().empty(), "null construction produces an empty string");
        expect(value.str() == expected, "construction follows the selected policy");
        expect(str_type::auto_trim == AutoTrim, "auto_trim reports the selected policy");
#ifdef JH_HEADER_NO_IMPL
        expect(str_type::is_static_built(), "static library mode is reported");
#else
        expect(!str_type::is_static_built(), "header-only mode is reported");
#endif
        expect(value.hash() == std::hash<std::string_view>{}(value.view()), "object hash uses stored content");

        shared_type first = jh::make_atomic<AutoTrim>(" \t hello \r ");
        shared_type second = jh::make_atomic<AutoTrim>(" \t hello \r ");
        weak_type weak = first;
        expect(first.get() != second.get() && weak.lock() == first, "shared and weak lifetime");
        expect(first->view() == expected, "factory follows the selected policy");

        jh::atomic_str_slot<AutoTrim> slot(first);
        jh::atomic_weak_str_slot<AutoTrim> weak_slot(weak);
        expect(slot.load() == first && weak_slot.load().lock() == first, "atomic slots retain the selected type");

        hash_type hash;
        eq_type eq;
        expect(eq(first, second), "different addresses with equal contents compare equal");
        expect(hash(first) == hash(second), "equal shared values hash equally");
        expect(eq(first, " \t hello \r ") && eq(" \t hello \r ", first), "literal comparison follows policy");
        expect(hash(first) == hash(" \t hello \r "), "equal literal and object hash equally");
        expect(eq(first, expected.c_str()) && hash(first) == hash(expected.c_str()), "stored content agrees with C string");
        expect(!eq(shared_type{}, first) && !eq(first, static_cast<const char *>(nullptr)), "null equality remains false");
        expect(hash(shared_type{}) == 0 && hash(nullptr) == 0, "null hash remains zero");

        shared_type empty = jh::make_atomic<AutoTrim>("");
        shared_type blank = jh::make_atomic<AutoTrim>(" \t\r ");
        expect(empty->view().empty(), "empty input remains empty");
        expect(blank->view() == (AutoTrim ? "" : " \t\r "), "all whitespace follows policy");
        expect(eq(blank, " \t\r ") && hash(blank) == hash(" \t\r "), "all whitespace hash and equality agree");

        std::unordered_set<shared_type, hash_type, eq_type> set;
        set.insert(first);
        expect(set.contains(" \t hello \r "), "unordered_set heterogeneous lookup");
        expect(set.contains(second), "unordered_set shared lookup");
        std::unordered_map<shared_type, int, hash_type, eq_type> map;
        map.emplace(first, 7);
        expect(map.find(" \t hello \r ") != map.end(), "unordered_map heterogeneous lookup");

        jh::observe_pool<str_type, pool_test_mutex> pool;
        const auto pooled1 = pool.acquire(" \t hello \r ");
        const auto pooled2 = pool.acquire(" \t hello \r ");
        expect(pooled1 == pooled2 && pooled1->view() == expected, "pool deduplicates stored content");

        std::mutex mutex;
        const std::string source = " \t hello \r ";
        shared_type safe = jh::safe_from<AutoTrim>(std::string_view(source), mutex);
        expect(safe->view() == expected, "locked factory follows policy");
        const std::string embedded_null{"x\0y", 3};
        jh::test::tiny_test::expect_throw<std::logic_error>(
            [&] { (void)jh::safe_from<AutoTrim>(std::string_view(embedded_null), mutex); },
            "locked factory rejects embedded nulls");
        auto safe_slot = jh::safe_from_slot<AutoTrim>(std::string_view(source), mutex);
        auto made_slot = jh::make_atomic_slot<AutoTrim>(source.c_str());
        expect(safe_slot.load()->view() == expected && made_slot.load()->view() == expected,
               "slot factories follow policy");
    }

    void policy_matrix() {
        using default_str = jh::immutable_str<>;
        static_assert(default_str::auto_trim == JH_IMMUTABLE_STR_AUTO_TRIM);
        static_assert(std::is_same_v<jh::shared_str_ptr<>, std::shared_ptr<default_str>>);
        static_assert(std::is_same_v<jh::weak_str_ptr<>, std::weak_ptr<default_str>>);
        static_assert(std::is_same_v<jh::atomic_str_slot<>, jh::atomic::shared_ptr<default_str>>);
        static_assert(std::is_same_v<jh::atomic_weak_str_slot<>, jh::atomic::weak_ptr<default_str>>);
        static_assert(std::is_same_v<jh::atomic_str_hash<>, jh::atomic_str_hash<JH_IMMUTABLE_STR_AUTO_TRIM>>);
        static_assert(std::is_same_v<jh::atomic_str_eq<>, jh::atomic_str_eq<JH_IMMUTABLE_STR_AUTO_TRIM>>);
        static_assert(!std::is_same_v<jh::immutable_str<true>, jh::immutable_str<false>>);
        static_assert(!std::is_same_v<jh::observe_pool<jh::immutable_str<true>, pool_test_mutex>,
                                      jh::observe_pool<jh::immutable_str<false>, pool_test_mutex>>);
        static_assert(!std::is_convertible_v<jh::shared_str_ptr<true>, jh::shared_str_ptr<false>>);
        static_assert(!std::is_invocable_v<jh::atomic_str_hash<true>, jh::shared_str_ptr<false>>);
        static_assert(!std::is_invocable_v<jh::atomic_str_eq<true>, jh::shared_str_ptr<true>, jh::shared_str_ptr<false>>);

        jh::immutable_str deduced{"hello"};
        jh::shared_str_ptr shared;
        jh::weak_str_ptr weak;
        jh::atomic_str_slot slot;
        jh::atomic_weak_str_slot weak_slot;
        jh::atomic_str_hash hash;
        jh::atomic_str_eq eq;
        static_assert(std::is_same_v<decltype(deduced), default_str>);
        static_assert(std::is_same_v<decltype(shared), jh::shared_str_ptr<>>);
        static_assert(std::is_same_v<decltype(weak), jh::weak_str_ptr<>>);
        static_assert(std::is_same_v<decltype(slot), jh::atomic_str_slot<>>);
        static_assert(std::is_same_v<decltype(weak_slot), jh::atomic_weak_str_slot<>>);
        static_assert(std::is_same_v<decltype(hash), jh::atomic_str_hash<>>);
        static_assert(std::is_same_v<decltype(eq), jh::atomic_str_eq<>>);
        static_assert(std::is_same_v<decltype(jh::make_atomic("hello")), jh::shared_str_ptr<>>);
        static_assert(std::is_same_v<decltype(jh::make_atomic_slot("hello")), jh::atomic_str_slot<>>);
        shared = jh::make_atomic("hello");
        weak = shared;
        slot.store(shared);
        weak_slot.store(weak);
        expect(deduced.view() == "hello" && weak.lock() == shared &&
                   slot.load() == shared && weak_slot.load().lock() == shared &&
                   eq(shared, "hello") && hash(shared) == hash("hello"),
               "all default-policy objects can be declared without angle brackets");
        expect(jh::make_atomic(" \t hello \r ")->view() ==
                   (JH_IMMUTABLE_STR_AUTO_TRIM ? "hello" : " \t hello \r "),
               "default factory follows macro");
        std::mutex mutex;
        std::string source = " \t hello \r ";
        auto default_safe = jh::safe_from(std::string_view(source), mutex);
        static_assert(std::is_same_v<decltype(default_safe), jh::shared_str_ptr<>>);
        static_assert(std::is_same_v<decltype(jh::safe_from<false>(std::string_view(source), mutex)),
                                     jh::shared_str_ptr<false>>);
        static_assert(std::is_same_v<decltype(jh::safe_from_slot<false>(std::string_view(source), mutex)),
                                     jh::atomic_str_slot<false>>);
        expect(default_safe->view() ==
                   (JH_IMMUTABLE_STR_AUTO_TRIM ? "hello" : " \t hello \r "),
               "default locked factory follows macro");
        auto default_slot = jh::make_atomic_slot(" \t hello \r ");
        auto default_safe_slot = jh::safe_from_slot(std::string_view(source), mutex);
        static_assert(std::is_same_v<decltype(default_slot), jh::atomic_str_slot<>>);
        static_assert(std::is_same_v<decltype(default_safe_slot), jh::atomic_str_slot<>>);
        expect(default_slot.load()->view() == default_safe_slot.load()->view(),
               "default slot factories share the configured policy");
        jh::observe_pool<default_str, pool_test_mutex> default_pool;
        expect(default_pool.acquire("hello") == default_pool.acquire("hello"),
               "default pool deduplicates strings");

        check_policy<JH_IMMUTABLE_STR_AUTO_TRIM>();
        check_policy<true>();
        check_policy<false>();
        jh::immutable_str<true> trim{" hello "};
        jh::immutable_str<false> raw{" hello "};
        expect(trim.view() == "hello" && raw.view() == " hello ", "both policies coexist");
    }
}

template<>
struct jh::test::tiny_test::test<"immutable_str policy matrix">
    : jh::test::tiny_test::test_definition<"immutable_str policy matrix", &::test::policy_matrix> {};

template<>
struct jh::test::tiny_test::session<"immutable_str policy module">
    : jh::test::tiny_test::session_definition<
          "immutable_str policy module", jh::test::tiny_test::test<"immutable_str policy matrix">> {};

namespace test {
    [[maybe_unused]] const jh::test::tiny_test::session<"immutable_str policy module"> registration{};
}
