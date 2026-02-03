#include <catch2/catch_all.hpp>

#include <random>
#include <vector>
#include <map>
#include <algorithm>

#include "jh/flat_multimap"

using jh::flat_multimap;

namespace helper{

    template<typename MM>
    std::vector<std::pair<int, int>> dump_multimap(const MM &mm) {
        std::vector<std::pair<int, int>> v;
        for (auto const &kv: mm)
            v.emplace_back(kv.first, kv.second);
        return v;
    }

} // namespace

TEST_CASE("basic insertion and ordering equivalence") {
    flat_multimap<int, int> fm;
    std::multimap<int, int> sm;

    std::vector<std::pair<int, int>> input = {
            {3, 30},
            {1, 10},
            {2, 20},
            {1, 11},
            {3, 31}
    };

    for (auto &p: input) {
        fm.insert(p);
        sm.insert(p);
    }

    REQUIRE(helper::dump_multimap(fm) == helper::dump_multimap(sm));
}

TEST_CASE("duplicate key equal_range behavior") {
    flat_multimap<int, int> fm;
    std::multimap<int, int> sm;

    for (int i = 0; i < 5; ++i) {
        fm.insert(std::forward_as_tuple(1, i));
        sm.insert({1, i});
    }

    auto [fl, fr] = fm.equal_range(1);
    auto [sl, sr] = sm.equal_range(1);

    std::vector<int> fv, sv;
    for (auto it = fl; it != fr; ++it) fv.push_back(it->second);
    for (auto it = sl; it != sr; ++it) sv.push_back(it->second);

    REQUIRE(fv == sv);
}

TEST_CASE("erase by key equivalence") {
    flat_multimap<int, int> fm;
    std::multimap<int, int> sm;

    for (int i = 0; i < 10; ++i) {
        fm.insert(std::forward_as_tuple(i % 3, i));
        sm.insert({i % 3, i});
    }

    auto fc = fm.erase(1);
    auto sc = sm.erase(1);

    REQUIRE(fc == sc);
    REQUIRE(helper::dump_multimap(fm) == helper::dump_multimap(sm));
}

TEST_CASE("find behavior equivalence") {
    flat_multimap<int, int> fm;
    std::multimap<int, int> sm;

    for (int i = 0; i < 20; ++i) {
        fm.insert(std::forward_as_tuple(i, i * 10));
        sm.insert({i, i * 10});
    }

    for (int k = -5; k <= 25; ++k) {
        auto fit = fm.find(k);
        auto sit = sm.find(k);

        if (sit == sm.end()) {
            REQUIRE(fit == fm.end());
        } else {
            REQUIRE(fit != fm.end());
            REQUIRE(fit->first == sit->first);
            REQUIRE(fit->second == sit->second);
        }
    }
}

TEST_CASE("random stress test flat_multimap vs std::multimap") {
    static std::mt19937 gen(123456); // NOLINT
    std::uniform_int_distribution<int> key_dist(0, 100);
    std::uniform_int_distribution<int> val_dist(0, 100000);

    flat_multimap<int, int> fm;
    std::multimap<int, int> sm;

    for (int i = 0; i < 20000; ++i) {
        int k = key_dist(gen);
        int v = val_dist(gen);

        fm.insert(std::forward_as_tuple(k, v));
        sm.insert({k, v});

        if (i % 7 == 0) {
            int ek = key_dist(gen);
            fm.erase(ek);
            sm.erase(ek);
        }
    }

    REQUIRE(helper::dump_multimap(fm) == helper::dump_multimap(sm));
}

TEST_CASE("range erase equivalence") {
    flat_multimap<int, int> fm;
    std::multimap<int, int> sm;

    for (int i = 0; i < 50; ++i) {
        fm.insert(std::forward_as_tuple(i / 5, i));
        sm.insert({i / 5, i});
    }

    auto [fl, fr] = fm.equal_range(5);
    auto [sl, sr] = sm.equal_range(5);

    fm.erase(fl, fr);
    sm.erase(sl, sr);

    REQUIRE(helper::dump_multimap(fm) == helper::dump_multimap(sm));
}

TEST_CASE("bulk construction then sort equivalence") {
    std::vector<std::pair<int, int>> data;
    data.reserve(100);
    for (int i = 0; i < 100; ++i)
        data.emplace_back(i % 10, i);

    flat_multimap<int, int> fm(data);
    std::multimap<int, int> sm(data.begin(), data.end());

    REQUIRE(helper::dump_multimap(fm) == helper::dump_multimap(sm));
}

TEST_CASE("emplace basic insertion equivalence") {
    flat_multimap<int, int> fm;
    std::multimap<int, int> sm;

    fm.emplace(3, 30);
    fm.emplace(1, 10);
    fm.emplace(2, 20);
    fm.emplace(1, 11);
    fm.emplace(3, 31);

    sm.emplace(3, 30);
    sm.emplace(1, 10);
    sm.emplace(2, 20);
    sm.emplace(1, 11);
    sm.emplace(3, 31);

    REQUIRE(helper::dump_multimap(fm) == helper::dump_multimap(sm));
}
