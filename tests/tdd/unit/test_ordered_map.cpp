#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

#include <memory_resource>
#include <random>
#include <ranges>
#include <vector>
#include <set>
#include <stdexcept>
#include "jh/ordered_map"

using jh::ordered_set;
using jh::ordered_map;


namespace test {
void tiny_test_case_1() {
    ordered_set<int> s;

    s.insert(5);
    s.insert(3);
    s.insert(7);
    s.insert(1);

    std::vector<int> v;
    for (auto &x: s) v.push_back(x);

    jh::test::tiny_test::expect(static_cast<bool>((v == std::vector<int>{1, 3, 5, 7})), "v == std::vector<int>{1, 3, 5, 7}");
    jh::test::tiny_test::expect(static_cast<bool>((s.count(5) == 1)), "s.count(5) == 1");
    jh::test::tiny_test::expect(static_cast<bool>((s.count(9) == 0)), "s.count(9) == 0");

}
}
template<>
struct jh::test::tiny_test::test<"basic set insert and iteration">
    : jh::test::tiny_test::test_definition<"basic set insert and iteration", &::test::tiny_test_case_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 1">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 1", jh::test::tiny_test::test<"basic set insert and iteration">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 1"> registration_1{};
}



namespace test {
void tiny_test_case_2() {
    ordered_map<int, int> mp;

    mp[3] = 30;
    mp[1] = 10;
    mp[2] = 20;
    mp[1] = 100;

    std::vector<std::pair<int, int>> v;
    for (auto &kv: mp)
        v.emplace_back(kv.first, kv.second);

    jh::test::tiny_test::expect(static_cast<bool>((v == std::vector<std::pair<int, int>>{
            {1, 100},
            {2, 20},
            {3, 30}
    })), "v == std::vector<std::pair<int, int>>{\n            {1, 100},\n            {2, 20},\n            {3, 30}\n    }");

    auto it = mp.find(2);
    jh::test::tiny_test::expect(static_cast<bool>((it != mp.end())), "it != mp.end()");
    jh::test::tiny_test::expect(static_cast<bool>((it->second == 20)), "it->second == 20");

}
}
template<>
struct jh::test::tiny_test::test<"basic map insert and operator[]">
    : jh::test::tiny_test::test_definition<"basic map insert and operator[]", &::test::tiny_test_case_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 2">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 2", jh::test::tiny_test::test<"basic map insert and operator[]">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 2"> registration_2{};
}



namespace test {
void tiny_test_case_3() {
    ordered_set<int> s;

    for (int i = 0; i < 10; i++) s.insert(i);

    s.erase(0);
    s.erase(5);
    s.erase(9);

    std::vector<int> v;
    for (auto &x: s) v.push_back(x);

    jh::test::tiny_test::expect(static_cast<bool>((v == std::vector<int>{1, 2, 3, 4, 6, 7, 8})), "v == std::vector<int>{1, 2, 3, 4, 6, 7, 8}");
    jh::test::tiny_test::expect(static_cast<bool>((s.count(5) == 0)), "s.count(5) == 0");
    jh::test::tiny_test::expect(static_cast<bool>((s.count(4) == 1)), "s.count(4) == 1");

}
}
template<>
struct jh::test::tiny_test::test<"set erase and iterator behavior">
    : jh::test::tiny_test::test_definition<"set erase and iterator behavior", &::test::tiny_test_case_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 3">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 3", jh::test::tiny_test::test<"set erase and iterator behavior">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 3"> registration_3{};
}



namespace test {
void tiny_test_case_4() {
    ordered_set<int> s;
    for (int i = 0; i <= 10; i += 2) s.insert(i);

    jh::test::tiny_test::expect(static_cast<bool>((*s.lower_bound(3) == 4)), "*s.lower_bound(3) == 4");
    jh::test::tiny_test::expect(static_cast<bool>((s.lower_bound(11) == s.end())), "s.lower_bound(11) == s.end()");

    jh::test::tiny_test::expect(static_cast<bool>((*s.upper_bound(4) == 6)), "*s.upper_bound(4) == 6");
    jh::test::tiny_test::expect(static_cast<bool>((s.upper_bound(10) == s.end())), "s.upper_bound(10) == s.end()");

    auto [l, r] = s.equal_range(4);
    jh::test::tiny_test::expect(static_cast<bool>((*l == 4)), "*l == 4");
    jh::test::tiny_test::expect(static_cast<bool>(((r == s.end() || *r == 6))), "(r == s.end() || *r == 6)");

}
}
template<>
struct jh::test::tiny_test::test<"bounds functions">
    : jh::test::tiny_test::test_definition<"bounds functions", &::test::tiny_test_case_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 4">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 4", jh::test::tiny_test::test<"bounds functions">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 4"> registration_4{};
}



namespace test {
void tiny_test_case_5() {
    ordered_set<int> s;
    for (int i = 1; i <= 5; i++) s.insert(i);

    std::vector<int> v;
    for (int it: std::ranges::reverse_view(s))
        v.push_back(it);

    jh::test::tiny_test::expect(static_cast<bool>((v == std::vector<int>{5, 4, 3, 2, 1})), "v == std::vector<int>{5, 4, 3, 2, 1}");

}
}
template<>
struct jh::test::tiny_test::test<"reverse iterator">
    : jh::test::tiny_test::test_definition<"reverse iterator", &::test::tiny_test_case_5> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 5">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 5", jh::test::tiny_test::test<"reverse iterator">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 5"> registration_5{};
}



namespace test {
void tiny_test_case_6() {
    ordered_set<int> s;
    for (int i = 0; i < 5; i++) s.insert(i);

    ordered_set<int> s2 = s;
    jh::test::tiny_test::expect(static_cast<bool>((s2.size() == 5)), "s2.size() == 5");

    ordered_set<int> s3 = std::move(s2);
    jh::test::tiny_test::expect(static_cast<bool>((s3.size() == 5)), "s3.size() == 5");

    std::vector<int> v;
    for (auto x: s3) v.push_back(x);
    jh::test::tiny_test::expect(static_cast<bool>((v == std::vector<int>{0, 1, 2, 3, 4})), "v == std::vector<int>{0, 1, 2, 3, 4}");

}
}
template<>
struct jh::test::tiny_test::test<"copy and move constructors">
    : jh::test::tiny_test::test_definition<"copy and move constructors", &::test::tiny_test_case_6> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 6">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 6", jh::test::tiny_test::test<"copy and move constructors">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 6"> registration_6{};
}




namespace test {
void tiny_test_case_7() {
    ordered_map<int, std::string> mp;

    auto [it1, ok1] = mp.emplace(1, "one");
    jh::test::tiny_test::expect(static_cast<bool>((ok1 == true)), "ok1 == true");
    jh::test::tiny_test::expect(static_cast<bool>((it1->first == 1)), "it1->first == 1");
    jh::test::tiny_test::expect(static_cast<bool>((it1->second == "one")), "it1->second == \"one\"");

    auto [it2, ok2] = mp.emplace(2, "two");
    jh::test::tiny_test::expect(static_cast<bool>((ok2 == true)), "ok2 == true");

    auto [it3, ok3] = mp.emplace(1, "xxx");
    jh::test::tiny_test::expect(static_cast<bool>((ok3 == false)), "ok3 == false");
    jh::test::tiny_test::expect(static_cast<bool>((it3->second == "one")), "it3->second == \"one\"");

    std::vector<std::pair<int, std::string>> v;
    for (auto &kv: mp)
        v.emplace_back(kv.first, kv.second);

    jh::test::tiny_test::expect(static_cast<bool>((v == std::vector<std::pair<int, std::string>>{
            {1, "one"},
            {2, "two"}
    })), "v == std::vector<std::pair<int, std::string>>{\n            {1, \"one\"},\n            {2, \"two\"}\n    }");

}
}
template<>
struct jh::test::tiny_test::test<"map emplace">
    : jh::test::tiny_test::test_definition<"map emplace", &::test::tiny_test_case_7> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 7">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 7", jh::test::tiny_test::test<"map emplace">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 7"> registration_7{};
}



namespace test {
void tiny_test_case_8() {
    ordered_set<int> s;

    auto [it1, ok1] = s.emplace(3);
    jh::test::tiny_test::expect(static_cast<bool>((ok1 == true)), "ok1 == true");

    auto [it2, ok2] = s.emplace(1);
    jh::test::tiny_test::expect(static_cast<bool>((ok2 == true)), "ok2 == true");

    auto [it3, ok3] = s.emplace(3);
    jh::test::tiny_test::expect(static_cast<bool>((ok3 == false)), "ok3 == false");

    std::vector<int> v;
    for (auto &x: s) v.push_back(x);

    jh::test::tiny_test::expect(static_cast<bool>((v == std::vector<int>{1, 3})), "v == std::vector<int>{1, 3}");

}
}
template<>
struct jh::test::tiny_test::test<"set emplace">
    : jh::test::tiny_test::test_definition<"set emplace", &::test::tiny_test_case_8> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 8">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 8", jh::test::tiny_test::test<"set emplace">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 8"> registration_8{};
}



namespace test {
void tiny_test_case_9() {
    ordered_map<int, int> mp;

    auto [it1, ok1] = mp.insert_or_assign(1, 10);
    jh::test::tiny_test::expect(static_cast<bool>((ok1 == true)), "ok1 == true");
    jh::test::tiny_test::expect(static_cast<bool>((it1->second == 10)), "it1->second == 10");

    auto [it2, ok2] = mp.insert_or_assign(2, 20);
    jh::test::tiny_test::expect(static_cast<bool>((ok2 == true)), "ok2 == true");

    auto [it3, ok3] = mp.insert_or_assign(1, 100);
    jh::test::tiny_test::expect(static_cast<bool>((ok3 == false)), "ok3 == false");
    jh::test::tiny_test::expect(static_cast<bool>((it3->second == 100)), "it3->second == 100");

    std::vector<std::pair<int, int>> v;
    for (auto &kv: mp)
        v.emplace_back(kv.first, kv.second);

    jh::test::tiny_test::expect(static_cast<bool>((v == std::vector<std::pair<int, int>>{
            {1, 100},
            {2, 20}
    })), "v == std::vector<std::pair<int, int>>{\n            {1, 100},\n            {2, 20}\n    }");

}
}
template<>
struct jh::test::tiny_test::test<"map insert_or_assign">
    : jh::test::tiny_test::test_definition<"map insert_or_assign", &::test::tiny_test_case_9> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 9">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 9", jh::test::tiny_test::test<"map insert_or_assign">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 9"> registration_9{};
}



namespace test {
void tiny_test_case_10() {
    for (int N = 1; N <= 200; N += 1) {

        std::vector<int> sorted;
        sorted.reserve(N);
        for (int i = 0; i < N; ++i)
            sorted.push_back(i);

        auto s = ordered_set<int>::from_sorted(sorted);

        jh::test::tiny_test::expect(static_cast<bool>((s.size() == (size_t) N)), "s.size() == (size_t) N");

        {
            std::vector<int> vec;
            vec.reserve(N);
            for (auto &x: s) vec.push_back(x);
            jh::test::tiny_test::expect(static_cast<bool>((vec == sorted)), "vec == sorted");
        }

        for (int i = 0; i < N; ++i) {
            auto it = s.find(i);
            jh::test::tiny_test::expect(static_cast<bool>((it != s.end())), "it != s.end()");
            jh::test::tiny_test::expect(static_cast<bool>((*it == i)), "*it == i");
        }
        jh::test::tiny_test::expect(static_cast<bool>((s.find(-1) == s.end())), "s.find(-1) == s.end()");
        jh::test::tiny_test::expect(static_cast<bool>((s.find(N + 1) == s.end())), "s.find(N + 1) == s.end()");

        for (int i = 0; i < N; ++i) {
            auto it = s.lower_bound(i);
            jh::test::tiny_test::expect(static_cast<bool>((it != s.end())), "it != s.end()");
            jh::test::tiny_test::expect(static_cast<bool>((*it == i)), "*it == i");
        }
        jh::test::tiny_test::expect(static_cast<bool>((s.lower_bound(N) == s.end())), "s.lower_bound(N) == s.end()");

        for (int i = 0; i < N - 1; ++i) {
            auto it = s.upper_bound(i);
            jh::test::tiny_test::expect(static_cast<bool>((it != s.end())), "it != s.end()");
            jh::test::tiny_test::expect(static_cast<bool>((*it == i + 1)), "*it == i + 1");
        }
        jh::test::tiny_test::expect(static_cast<bool>((s.upper_bound(N - 1) == s.end())), "s.upper_bound(N - 1) == s.end()");

        for (int i = 0; i < N; ++i) {
            auto [l, r] = s.equal_range(i);
            jh::test::tiny_test::expect(static_cast<bool>((l != s.end())), "l != s.end()");
            jh::test::tiny_test::expect(static_cast<bool>((*l == i)), "*l == i");
            if (i + 1 < N) {
                jh::test::tiny_test::expect(static_cast<bool>((r != s.end())), "r != s.end()");
                jh::test::tiny_test::expect(static_cast<bool>((*r == i + 1)), "*r == i + 1");
            } else {
                jh::test::tiny_test::expect(static_cast<bool>((r == s.end())), "r == s.end()");
            }
        }

        {
            int current = N - 1;
            for (int it: std::ranges::reverse_view(s)) {
                jh::test::tiny_test::expect(static_cast<bool>((it == current)), "it == current");
                --current;
            }
            jh::test::tiny_test::expect(static_cast<bool>((current == -1)), "current == -1");
        }

        if (N > 3) {
            auto x = N / 2;
            jh::test::tiny_test::expect(static_cast<bool>((s.erase(x) == 1)), "s.erase(x) == 1");
            jh::test::tiny_test::expect(static_cast<bool>((s.find(x) == s.end())), "s.find(x) == s.end()");

            std::vector<int> v2;
            for (auto &x2: s) v2.push_back(x2);

            sorted.erase(sorted.begin() + x);
            jh::test::tiny_test::expect(static_cast<bool>((v2 == sorted)), "v2 == sorted");
        }
    }

}
}
template<>
struct jh::test::tiny_test::test<"from_sorted basic ordering and lookup">
    : jh::test::tiny_test::test_definition<"from_sorted basic ordering and lookup", &::test::tiny_test_case_10> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 10">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 10", jh::test::tiny_test::test<"from_sorted basic ordering and lookup">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 10"> registration_10{};
}



namespace test {
void tiny_test_case_11() {
    ordered_map<int, std::string> mp;
    // 1) std::pair<K, V>
    {
        std::pair<int, std::string> p{1, "one"};
        auto [it, ok] = mp.insert(p);
        jh::test::tiny_test::expect(static_cast<bool>((ok == true)), "ok == true");
        jh::test::tiny_test::expect(static_cast<bool>((it->first == 1)), "it->first == 1");
        jh::test::tiny_test::expect(static_cast<bool>((it->second == "one")), "it->second == \"one\"");
    }
    // 2) std::pair<const K, V>
    {
        std::pair<const int, std::string> p{2, "two"};
        auto [it, ok] = mp.insert(p);
        jh::test::tiny_test::expect(static_cast<bool>((ok == true)), "ok == true");
        jh::test::tiny_test::expect(static_cast<bool>((it->first == 2)), "it->first == 2");
        jh::test::tiny_test::expect(static_cast<bool>((it->second == "two")), "it->second == \"two\"");
    }
    // 3) std::pair<K, const V>
    {
        const std::string s = "three";
        std::pair<int, const std::string> p{3, s};
        auto [it, ok] = mp.insert(p);
        jh::test::tiny_test::expect(static_cast<bool>((ok == true)), "ok == true");
        jh::test::tiny_test::expect(static_cast<bool>((it->first == 3)), "it->first == 3");
        jh::test::tiny_test::expect(static_cast<bool>((it->second == "three")), "it->second == \"three\"");
    }
    // 4) std::pair<const K, const V>
    {
        const std::string s = "four";
        const std::pair<const int, const std::string> p{4, s};
        auto [it, ok] = mp.insert(p);
        jh::test::tiny_test::expect(static_cast<bool>((ok == true)), "ok == true");
        jh::test::tiny_test::expect(static_cast<bool>((it->first == 4)), "it->first == 4");
        jh::test::tiny_test::expect(static_cast<bool>((it->second == "four")), "it->second == \"four\"");
    }
    // 5) std::tuple<K, V>
    {
        auto tup = std::make_tuple(5, std::string("five"));
        auto [it, ok] = mp.insert(tup);
        jh::test::tiny_test::expect(static_cast<bool>((ok == true)), "ok == true");
        jh::test::tiny_test::expect(static_cast<bool>((it->first == 5)), "it->first == 5");
        jh::test::tiny_test::expect(static_cast<bool>((it->second == "five")), "it->second == \"five\"");
    }
    std::vector<std::pair<int, std::string>> v;
    for (auto &kv: mp)
        v.emplace_back(kv.first, kv.second);

    jh::test::tiny_test::expect(static_cast<bool>((v == std::vector<std::pair<int, std::string>>{
            {1, "one"},
            {2, "two"},
            {3, "three"},
            {4, "four"},
            {5, "five"}
    })), "v == std::vector<std::pair<int, std::string>>{\n            {1, \"one\"},\n            {2, \"two\"},\n            {3, \"three\"},\n            {4, \"four\"},\n            {5, \"five\"}\n    }");

}
}
template<>
struct jh::test::tiny_test::test<"map insert with various pair-like types">
    : jh::test::tiny_test::test_definition<"map insert with various pair-like types", &::test::tiny_test_case_11> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 11">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 11", jh::test::tiny_test::test<"map insert with various pair-like types">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 11"> registration_11{};
}



namespace test {
void tiny_test_case_12() {
    std::vector<std::pair<int, std::string>> input = {
            {3, "ccc"},
            {1, "aaa"},
            {2, "bbb"},
            {1, "ignored duplicate"},
            {4, "ddd"}
    };

    ordered_map<int, std::string> mp(input);

    std::vector<std::pair<int, std::string>> out;
    out.reserve(mp.size());
    for (auto &kv: mp)
        out.emplace_back(kv.first, kv.second);

    jh::test::tiny_test::expect(static_cast<bool>((out == std::vector<std::pair<int, std::string>>{
            {1, "aaa"},
            {2, "bbb"},
            {3, "ccc"},
            {4, "ddd"}
    })), "out == std::vector<std::pair<int, std::string>>{\n            {1, \"aaa\"},\n            {2, \"bbb\"},\n            {3, \"ccc\"},\n            {4, \"ddd\"}\n    }");

}
}
template<>
struct jh::test::tiny_test::test<"map range construction follows insert semantics">
    : jh::test::tiny_test::test_definition<"map range construction follows insert semantics", &::test::tiny_test_case_12> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 12">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 12", jh::test::tiny_test::test<"map range construction follows insert semantics">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 12"> registration_12{};
}



namespace test {
void tiny_test_case_13() {
    using T = std::tuple<int, std::string>;

    std::vector<T> vec = {
            {3, "ccc"},
            {1, "aaa"},
            {2, "bbb"},
            {2, "ZZZ"},
            {4, "ddd"}
    };

    std::sort(vec.begin(), vec.end(), [](auto const &a, auto const &b) {
        return std::get<0>(a) < std::get<0>(b);
    });

    vec.erase(std::unique(vec.begin(), vec.end(), [](auto const &a, auto const &b) {
        return std::get<0>(a) == std::get<0>(b);
    }), vec.end());

    jh::test::tiny_test::expect(static_cast<bool>((vec.size() == 4)), "vec.size() == 4");
    jh::test::tiny_test::expect(static_cast<bool>((std::get<0>(vec[0]) == 1)), "std::get<0>(vec[0]) == 1");
    jh::test::tiny_test::expect(static_cast<bool>((std::get<0>(vec[1]) == 2)), "std::get<0>(vec[1]) == 2");

    auto mp = ordered_map<int, std::string>::from_sorted(vec);
    jh::test::tiny_test::expect(static_cast<bool>((mp.size() == 4)), "mp.size() == 4");

    std::vector<std::pair<int, std::string>> out;
    for (auto &kv: mp) out.emplace_back(kv.first, kv.second);

    jh::test::tiny_test::expect(static_cast<bool>((out == std::vector<std::pair<int, std::string>>{
            {1, "aaa"},
            {2, "bbb"},
            {3, "ccc"},
            {4, "ddd"}
    })), "out == std::vector<std::pair<int, std::string>>{\n            {1, \"aaa\"},\n            {2, \"bbb\"},\n            {3, \"ccc\"},\n            {4, \"ddd\"}\n    }");

    for (int i = 1; i <= 4; i++) {
        auto it = mp.find(i);
        jh::test::tiny_test::expect(static_cast<bool>((it != mp.end())), "it != mp.end()");
        jh::test::tiny_test::expect(static_cast<bool>((it->first == i)), "it->first == i");
    }
    jh::test::tiny_test::expect(static_cast<bool>((mp.find(0) == mp.end())), "mp.find(0) == mp.end()");
    jh::test::tiny_test::expect(static_cast<bool>((mp.find(5) == mp.end())), "mp.find(5) == mp.end()");

}
}
template<>
struct jh::test::tiny_test::test<"map from_sorted with tuple<K,V> input">
    : jh::test::tiny_test::test_definition<"map from_sorted with tuple<K,V> input", &::test::tiny_test_case_13> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 13">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 13", jh::test::tiny_test::test<"map from_sorted with tuple<K,V> input">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 13"> registration_13{};
}



namespace test {
void tiny_test_case_14_body(const int selected_section) {

    if (selected_section == 1) {
        ordered_set<int> s;
        jh::test::tiny_test::expect(static_cast<bool>((s.empty())), "s.empty()");
        jh::test::tiny_test::expect(static_cast<bool>((s.size() == 0)), "s.size() == 0"); // NOLINT

        for (int i = 0; i < 10; i++) s.insert(i);

        jh::test::tiny_test::expect(static_cast<bool>((!s.empty())), "!s.empty()");
        jh::test::tiny_test::expect(static_cast<bool>((s.size() == 10)), "s.size() == 10");

        s.clear();
        jh::test::tiny_test::expect(static_cast<bool>((s.empty())), "s.empty()");
        jh::test::tiny_test::expect(static_cast<bool>((s.size() == 0)), "s.size() == 0"); // NOLINT

        s.insert(42);
        jh::test::tiny_test::expect(static_cast<bool>((s.size() == 1)), "s.size() == 1");
        jh::test::tiny_test::expect(static_cast<bool>((!s.empty())), "!s.empty()");
    }

    if (selected_section == 2) {
        ordered_map<int, int> m;
        jh::test::tiny_test::expect(static_cast<bool>((m.empty())), "m.empty()");
        jh::test::tiny_test::expect(static_cast<bool>((m.size() == 0)), "m.size() == 0"); // NOLINT
        m[1] = 10;
        m[2] = 20;
        jh::test::tiny_test::expect(static_cast<bool>((!m.empty())), "!m.empty()");
        jh::test::tiny_test::expect(static_cast<bool>((m.size() == 2)), "m.size() == 2");

        m.clear();
        jh::test::tiny_test::expect(static_cast<bool>((m.empty())), "m.empty()");
        jh::test::tiny_test::expect(static_cast<bool>((m.size() == 0)), "m.size() == 0"); // NOLINT

        m[5] = 50;
        jh::test::tiny_test::expect(static_cast<bool>((m.size() == 1)), "m.size() == 1");
    }

    if (selected_section == 3) {
        ordered_set<int> s;
        for (int i = 1; i <= 5; i++) s.insert(i);

        s.reserve(1000);
        jh::test::tiny_test::expect(static_cast<bool>((s.size() == 5)), "s.size() == 5");

        std::vector<int> v;
        for (auto x: s) v.push_back(x);
        jh::test::tiny_test::expect(static_cast<bool>((v == std::vector<int>{1, 2, 3, 4, 5})), "v == std::vector<int>{1, 2, 3, 4, 5}");
    }

    if (selected_section == 4) {
        ordered_map<int, int> m;
        m[1] = 10;
        m[2] = 20;
        m[3] = 30;

        m.reserve(500);

        std::vector<std::pair<int, int>> v;
        for (auto &kv: m) v.emplace_back(kv.first, kv.second);

        jh::test::tiny_test::expect(static_cast<bool>((v == std::vector<std::pair<int, int>>{
                {1, 10},
                {2, 20},
                {3, 30}
        })), "v == std::vector<std::pair<int, int>>{\n                {1, 10},\n                {2, 20},\n                {3, 30}\n        }");
    }

    if (selected_section == 5) {
        ordered_set<int> s;
        for (int i = 10; i <= 50; i += 10) s.insert(i);

        s.shrink_to_fit();

        jh::test::tiny_test::expect(static_cast<bool>((s.size() == 5)), "s.size() == 5");

        std::vector<int> v;
        for (auto x: s) v.push_back(x);
        jh::test::tiny_test::expect(static_cast<bool>((v == std::vector<int>{10, 20, 30, 40, 50})), "v == std::vector<int>{10, 20, 30, 40, 50}");
    }

    if (selected_section == 6) {
        ordered_map<int, std::string> mp;
        mp.emplace(1, "a");
        mp.emplace(2, "b");
        mp.emplace(3, "c");

        mp.shrink_to_fit();

        std::vector<std::pair<int, std::string>> v;
        for (auto &kv: mp)
            v.emplace_back(kv.first, kv.second);

        jh::test::tiny_test::expect(static_cast<bool>((v == std::vector<std::pair<int, std::string>>{
                {1, "a"},
                {2, "b"},
                {3, "c"}
        })), "v == std::vector<std::pair<int, std::string>>{\n                {1, \"a\"},\n                {2, \"b\"},\n                {3, \"c\"}\n        }");
    }

    if (selected_section == 7) {
        ordered_map<int, int> mp;
        mp[1] = 10;
        jh::test::tiny_test::expect(static_cast<bool>((mp.at(1) == 10)), "mp.at(1) == 10");
        jh::test::tiny_test::expect_throw<std::out_of_range>([&]() { (void)(mp.at(2)); }, "throws std::out_of_range: mp.at(2)");
    }

}
void tiny_test_case_14_section_1() { tiny_test_case_14_body(1); }
void tiny_test_case_14_section_2() { tiny_test_case_14_body(2); }
void tiny_test_case_14_section_3() { tiny_test_case_14_body(3); }
void tiny_test_case_14_section_4() { tiny_test_case_14_body(4); }
void tiny_test_case_14_section_5() { tiny_test_case_14_body(5); }
void tiny_test_case_14_section_6() { tiny_test_case_14_body(6); }
void tiny_test_case_14_section_7() { tiny_test_case_14_body(7); }
}
template<>
struct jh::test::tiny_test::test<"container capacity-related utility functions / ordered_set basic size/empty and clear behavior">
    : jh::test::tiny_test::test_definition<"container capacity-related utility functions / ordered_set basic size/empty and clear behavior", &::test::tiny_test_case_14_section_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 14">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 14", jh::test::tiny_test::test<"container capacity-related utility functions / ordered_set basic size/empty and clear behavior">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 14"> registration_14{};
}
template<>
struct jh::test::tiny_test::test<"container capacity-related utility functions / ordered_map basic size/empty and clear behavior">
    : jh::test::tiny_test::test_definition<"container capacity-related utility functions / ordered_map basic size/empty and clear behavior", &::test::tiny_test_case_14_section_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 15">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 15", jh::test::tiny_test::test<"container capacity-related utility functions / ordered_map basic size/empty and clear behavior">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 15"> registration_15{};
}
template<>
struct jh::test::tiny_test::test<"container capacity-related utility functions / reserve does not affect size or contents">
    : jh::test::tiny_test::test_definition<"container capacity-related utility functions / reserve does not affect size or contents", &::test::tiny_test_case_14_section_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 16">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 16", jh::test::tiny_test::test<"container capacity-related utility functions / reserve does not affect size or contents">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 16"> registration_16{};
}
template<>
struct jh::test::tiny_test::test<"container capacity-related utility functions / reserve for map preserves elements">
    : jh::test::tiny_test::test_definition<"container capacity-related utility functions / reserve for map preserves elements", &::test::tiny_test_case_14_section_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 17">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 17", jh::test::tiny_test::test<"container capacity-related utility functions / reserve for map preserves elements">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 17"> registration_17{};
}
template<>
struct jh::test::tiny_test::test<"container capacity-related utility functions / shrink_to_fit does not change size or ordering">
    : jh::test::tiny_test::test_definition<"container capacity-related utility functions / shrink_to_fit does not change size or ordering", &::test::tiny_test_case_14_section_5> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 18">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 18", jh::test::tiny_test::test<"container capacity-related utility functions / shrink_to_fit does not change size or ordering">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 18"> registration_18{};
}
template<>
struct jh::test::tiny_test::test<"container capacity-related utility functions / shrink_to_fit for map preserves structure">
    : jh::test::tiny_test::test_definition<"container capacity-related utility functions / shrink_to_fit for map preserves structure", &::test::tiny_test_case_14_section_6> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 19">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 19", jh::test::tiny_test::test<"container capacity-related utility functions / shrink_to_fit for map preserves structure">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 19"> registration_19{};
}
template<>
struct jh::test::tiny_test::test<"container capacity-related utility functions / map at() throws for missing keys">
    : jh::test::tiny_test::test_definition<"container capacity-related utility functions / map at() throws for missing keys", &::test::tiny_test_case_14_section_7> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 20">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 20", jh::test::tiny_test::test<"container capacity-related utility functions / map at() throws for missing keys">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 20"> registration_20{};
}



namespace test {
void tiny_test_case_15() {
    ordered_map<std::string, int> mp;

    mp.emplace("banana", 1);
    mp.emplace("apple", 2);
    mp.emplace("date", 3);
    mp.emplace("cherry", 4);

    jh::test::tiny_test::expect(static_cast<bool>((mp.size() == 4)), "mp.size() == 4");

    std::vector<std::pair<std::string, int>> from_map;
    for (const auto &[k, v]: mp)
        from_map.emplace_back(k, v);

    std::vector<std::pair<std::string, int>> expected = {
            {"banana", 1},
            {"apple",  2},
            {"date",   3},
            {"cherry", 4}
    };

    std::stable_sort(expected.begin(), expected.end(),
                     [](const auto &a, const auto &b) {
                         return a.first < b.first;
                     });

    jh::test::tiny_test::expect(static_cast<bool>((from_map == expected)), "from_map == expected");

}
}
template<>
struct jh::test::tiny_test::test<"string ordered_map basic ordering and uniqueness">
    : jh::test::tiny_test::test_definition<"string ordered_map basic ordering and uniqueness", &::test::tiny_test_case_15> {};
template<>
struct jh::test::tiny_test::session<"test module test_ordered_map case 21">
    : jh::test::tiny_test::session_definition<
          "test module test_ordered_map case 21", jh::test::tiny_test::test<"string ordered_map basic ordering and uniqueness">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_ordered_map case 21"> registration_21{};
}


