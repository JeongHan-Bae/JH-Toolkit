#include <catch2/catch_all.hpp>

#include <memory_resource>
#include <random>
#include <ranges>
#include <vector>
#include <set>
#include "jh/ordered_map"

using jh::ordered_set;
using jh::ordered_map;

TEST_CASE("range check") {
    STATIC_REQUIRE(std::ranges::bidirectional_range<ordered_set<int>>);
    STATIC_REQUIRE(std::ranges::bidirectional_range<ordered_map<int, int>>);
}

TEST_CASE("basic set insert and iteration") {
    ordered_set<int> s;

    s.insert(5);
    s.insert(3);
    s.insert(7);
    s.insert(1);

    std::vector<int> v;
    for (auto &x: s) v.push_back(x);

    REQUIRE(v == std::vector<int>{1, 3, 5, 7});
    REQUIRE(s.count(5) == 1);
    REQUIRE(s.count(9) == 0);
}

TEST_CASE("basic map insert and operator[]") {
    ordered_map<int, int> mp;

    mp[3] = 30;
    mp[1] = 10;
    mp[2] = 20;
    mp[1] = 100;

    std::vector<std::pair<int, int>> v;
    for (auto &kv: mp)
        v.emplace_back(kv.first, kv.second);

    REQUIRE(v == std::vector<std::pair<int, int>>{
            {1, 100},
            {2, 20},
            {3, 30}
    });

    auto it = mp.find(2);
    REQUIRE(it != mp.end());
    REQUIRE(it->second == 20);
}

TEST_CASE("set erase and iterator behavior") {
    ordered_set<int> s;

    for (int i = 0; i < 10; i++) s.insert(i);

    s.erase(0);
    s.erase(5);
    s.erase(9);

    std::vector<int> v;
    for (auto &x: s) v.push_back(x);

    REQUIRE(v == std::vector<int>{1, 2, 3, 4, 6, 7, 8});
    REQUIRE(s.count(5) == 0);
    REQUIRE(s.count(4) == 1);
}

TEST_CASE("bounds functions") {
    ordered_set<int> s;
    for (int i = 0; i <= 10; i += 2) s.insert(i);

    REQUIRE(*s.lower_bound(3) == 4);
    REQUIRE(s.lower_bound(11) == s.end());

    REQUIRE(*s.upper_bound(4) == 6);
    REQUIRE(s.upper_bound(10) == s.end());

    auto [l, r] = s.equal_range(4);
    REQUIRE(*l == 4);
    REQUIRE((r == s.end() || *r == 6));
}

TEST_CASE("reverse iterator") {
    ordered_set<int> s;
    for (int i = 1; i <= 5; i++) s.insert(i);

    std::vector<int> v;
    for (int it: std::ranges::reverse_view(s))
        v.push_back(it);

    REQUIRE(v == std::vector<int>{5, 4, 3, 2, 1});
}

TEST_CASE("copy and move constructors") {
    ordered_set<int> s;
    for (int i = 0; i < 5; i++) s.insert(i);

    ordered_set<int> s2 = s;
    REQUIRE(s2.size() == 5);

    ordered_set<int> s3 = std::move(s2);
    REQUIRE(s3.size() == 5);

    std::vector<int> v;
    for (auto x: s3) v.push_back(x);
    REQUIRE(v == std::vector<int>{0, 1, 2, 3, 4});
}

TEST_CASE("random stress test vs std::set") {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 100000);

    ordered_set<int> s;
    std::set<int> stds;

    for (int i = 0; i < 20000; i++) {
        int x = dist(gen);
        s.insert(x);
        stds.insert(x);

        if (i % 10 == 0) {
            int y = dist(gen);
            s.erase(y);
            stds.erase(y);
        }
    }

    std::vector<int> a, b;
    for (auto x: s) a.push_back(x);
    for (auto x: stds) b.push_back(x);

    REQUIRE(a == b);
}

TEST_CASE("map emplace") {
    ordered_map<int, std::string> mp;

    auto [it1, ok1] = mp.emplace(1, "one");
    REQUIRE(ok1 == true);
    REQUIRE(it1->first == 1);
    REQUIRE(it1->second == "one");

    auto [it2, ok2] = mp.emplace(2, "two");
    REQUIRE(ok2 == true);

    auto [it3, ok3] = mp.emplace(1, "xxx");
    REQUIRE(ok3 == false);
    REQUIRE(it3->second == "one");

    std::vector<std::pair<int,std::string>> v;
    for (auto &kv : mp)
        v.emplace_back(kv.first, kv.second);

    REQUIRE(v == std::vector<std::pair<int,std::string>>{
            {1,"one"}, {2,"two"}
    });
}

TEST_CASE("set emplace") {
    ordered_set<int> s;

    auto [it1, ok1] = s.emplace(3);
    REQUIRE(ok1 == true);

    auto [it2, ok2] = s.emplace(1);
    REQUIRE(ok2 == true);

    auto [it3, ok3] = s.emplace(3);
    REQUIRE(ok3 == false);

    std::vector<int> v;
    for (auto &x : s) v.push_back(x);

    REQUIRE(v == std::vector<int>{1, 3});
}

TEST_CASE("map insert_or_assign") {
    ordered_map<int, int> mp;

    auto [it1, ok1] = mp.insert_or_assign(1, 10);
    REQUIRE(ok1 == true);
    REQUIRE(it1->second == 10);

    auto [it2, ok2] = mp.insert_or_assign(2, 20);
    REQUIRE(ok2 == true);

    auto [it3, ok3] = mp.insert_or_assign(1, 100);
    REQUIRE(ok3 == false);
    REQUIRE(it3->second == 100);

    std::vector<std::pair<int,int>> v;
    for (auto &kv : mp)
        v.emplace_back(kv.first, kv.second);

    REQUIRE(v == std::vector<std::pair<int,int>>{
            {1, 100}, {2, 20}
    });
}

TEST_CASE("from_sorted basic ordering and lookup") {
    for (int N = 1; N <= 200; N+=1) {

        std::vector<int> sorted;
        sorted.reserve(N);
        for (int i = 0; i < N; ++i)
            sorted.push_back(i);

        auto s = ordered_set<int>::from_sorted(sorted);

        REQUIRE(s.size() == (size_t)N);

        {
            std::vector<int> vec;
            vec.reserve(N);
            for (auto &x : s) vec.push_back(x);
            REQUIRE(vec == sorted);
        }

        for (int i = 0; i < N; ++i) {
            auto it = s.find(i);
            REQUIRE(it != s.end());
            REQUIRE(*it == i);
        }
        REQUIRE(s.find(-1) == s.end());
        REQUIRE(s.find(N+1) == s.end());

        for (int i = 0; i < N; ++i) {
            auto it = s.lower_bound(i);
            REQUIRE(it != s.end());
            REQUIRE(*it == i);
        }
        REQUIRE(s.lower_bound(N) == s.end());

        for (int i = 0; i < N - 1; ++i) {
            auto it = s.upper_bound(i);
            REQUIRE(it != s.end());
            REQUIRE(*it == i + 1);
        }
        REQUIRE(s.upper_bound(N - 1) == s.end());

        for (int i = 0; i < N; ++i) {
            auto [l, r] = s.equal_range(i);
            REQUIRE(l != s.end());
            REQUIRE(*l == i);
            if (i + 1 < N) {
                REQUIRE(r != s.end());
                REQUIRE(*r == i + 1);
            } else {
                REQUIRE(r == s.end());
            }
        }

        {
            int current = N - 1;
            for (int it : std::ranges::reverse_view(s)) {
                REQUIRE(it == current);
                --current;
            }
            REQUIRE(current == -1);
        }

        if (N > 3) {
            auto x = N / 2;
            REQUIRE(s.erase(x) == 1);
            REQUIRE(s.find(x) == s.end());

            std::vector<int> v2;
            for (auto &x2 : s) v2.push_back(x2);

            sorted.erase(sorted.begin() + x);
            REQUIRE(v2 == sorted);
        }
    }
}

TEST_CASE("map insert with various pair-like types") {
    ordered_map<int, std::string> mp;
    // 1) std::pair<K, V>
    {
        std::pair<int, std::string> p{1, "one"};
        auto [it, ok] = mp.insert(p);
        REQUIRE(ok == true);
        REQUIRE(it->first == 1);
        REQUIRE(it->second == "one");
    }
    // 2) std::pair<const K, V>
    {
        std::pair<const int, std::string> p{2, "two"};
        auto [it, ok] = mp.insert(p);
        REQUIRE(ok == true);
        REQUIRE(it->first == 2);
        REQUIRE(it->second == "two");
    }
    // 3) std::pair<K, const V>
    {
        const std::string s = "three";
        std::pair<int, const std::string> p{3, s};
        auto [it, ok] = mp.insert(p);
        REQUIRE(ok == true);
        REQUIRE(it->first == 3);
        REQUIRE(it->second == "three");
    }
    // 4) std::pair<const K, const V>
    {
        const std::string s = "four";
        const std::pair<const int, const std::string> p{4, s};
        auto [it, ok] = mp.insert(p);
        REQUIRE(ok == true);
        REQUIRE(it->first == 4);
        REQUIRE(it->second == "four");
    }
    // 5) std::tuple<K, V>
    {
        auto tup = std::make_tuple(5, std::string("five"));
        auto [it, ok] = mp.insert(tup);
        REQUIRE(ok == true);
        REQUIRE(it->first == 5);
        REQUIRE(it->second == "five");
    }
    std::vector<std::pair<int, std::string>> v;
    for (auto &kv : mp)
        v.emplace_back(kv.first, kv.second);

    REQUIRE(v == std::vector<std::pair<int, std::string>>{
            {1,"one"},
            {2,"two"},
            {3,"three"},
            {4,"four"},
            {5,"five"}
    });
}

TEST_CASE("map from_sorted with tuple<K,V> input") {
    using T = std::tuple<int, std::string>;

    std::vector<T> vec = {
            {3, "ccc"},
            {1, "aaa"},
            {2, "bbb"},
            {2, "ZZZ"},
            {4, "ddd"}
    };

    std::sort(vec.begin(), vec.end(), [](auto const& a, auto const& b) {
        return std::get<0>(a) < std::get<0>(b);
    });

    vec.erase(std::unique(vec.begin(), vec.end(), [](auto const& a, auto const& b) {
        return std::get<0>(a) == std::get<0>(b);
    }), vec.end());

    REQUIRE(vec.size() == 4);
    REQUIRE(std::get<0>(vec[0]) == 1);
    REQUIRE(std::get<0>(vec[1]) == 2);

    auto mp = ordered_map<int, std::string>::from_sorted(vec);
    REQUIRE(mp.size() == 4);

    std::vector<std::pair<int,std::string>> out;
    for (auto& kv : mp) out.emplace_back(kv.first, kv.second);

    REQUIRE(out == std::vector<std::pair<int,std::string>>{
            {1,"aaa"},
            {2,"bbb"},
            {3,"ccc"},
            {4,"ddd"}
    });

    for (int i = 1; i <= 4; i++) {
        auto it = mp.find(i);
        REQUIRE(it != mp.end());
        REQUIRE(it->first == i);
    }
    REQUIRE(mp.find(0) == mp.end());
    REQUIRE(mp.find(5) == mp.end());
}

TEST_CASE("container capacity-related utility functions") {

    SECTION("ordered_set basic size/empty and clear behavior") {
        ordered_set<int> s;
        REQUIRE(s.empty());
        REQUIRE(s.size() == 0); // NOLINT

        for (int i = 0; i < 10; i++) s.insert(i);

        REQUIRE(!s.empty());
        REQUIRE(s.size() == 10);

        s.clear();
        REQUIRE(s.empty());
        REQUIRE(s.size() == 0); // NOLINT

        s.insert(42);
        REQUIRE(s.size() == 1);
        REQUIRE(!s.empty());
    }

    SECTION("ordered_map basic size/empty and clear behavior") {
        ordered_map<int, int> m;
        REQUIRE(m.empty());
        REQUIRE(m.size() == 0); // NOLINT
        m[1] = 10;
        m[2] = 20;
        REQUIRE(!m.empty());
        REQUIRE(m.size() == 2);

        m.clear();
        REQUIRE(m.empty());
        REQUIRE(m.size() == 0); // NOLINT

        m[5] = 50;
        REQUIRE(m.size() == 1);
    }

    SECTION("reserve does not affect size or contents") {
        ordered_set<int> s;
        for (int i = 1; i <= 5; i++) s.insert(i);

        s.reserve(1000);
        REQUIRE(s.size() == 5);

        std::vector<int> v;
        for (auto x : s) v.push_back(x);
        REQUIRE(v == std::vector<int>{1,2,3,4,5});
    }

    SECTION("reserve for map preserves elements") {
        ordered_map<int, int> m;
        m[1] = 10;
        m[2] = 20;
        m[3] = 30;

        m.reserve(500);

        std::vector<std::pair<int,int>> v;
        for (auto& kv : m) v.emplace_back(kv.first, kv.second);

        REQUIRE(v == std::vector<std::pair<int,int>>{
                {1,10},{2,20},{3,30}
        });
    }

    SECTION("shrink_to_fit does not change size or ordering") {
        ordered_set<int> s;
        for (int i = 10; i <= 50; i += 10) s.insert(i);

        s.shrink_to_fit();

        REQUIRE(s.size() == 5);

        std::vector<int> v;
        for (auto x : s) v.push_back(x);
        REQUIRE(v == std::vector<int>{10,20,30,40,50});
    }

    SECTION("shrink_to_fit for map preserves structure") {
        ordered_map<int, std::string> mp;
        mp.emplace(1, "a");
        mp.emplace(2, "b");
        mp.emplace(3, "c");

        mp.shrink_to_fit();

        std::vector<std::pair<int,std::string>> v;
        for (auto& kv : mp) v.emplace_back(kv.first, kv.second);

        REQUIRE(v == std::vector<std::pair<int,std::string>>{
                {1,"a"}, {2,"b"}, {3,"c"}
        });
    }
}
