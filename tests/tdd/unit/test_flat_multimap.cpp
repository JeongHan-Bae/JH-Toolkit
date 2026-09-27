#define TINY_TEST_MAIN
#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

#include <random>
#include <vector>
#include <map>
#include <algorithm>
#include <stdexcept>

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


namespace test {
void tiny_test_case_1() {
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

    jh::test::tiny_test::expect(static_cast<bool>((helper::dump_multimap(fm) == helper::dump_multimap(sm))), "helper::dump_multimap(fm) == helper::dump_multimap(sm)");

}
}
template<>
struct jh::test::tiny_test::test<"basic insertion and ordering equivalence">
    : jh::test::tiny_test::test_definition<"basic insertion and ordering equivalence", &::test::tiny_test_case_1> {};
template<>
struct jh::test::tiny_test::session<"test module test_flat_multimap case 1">
    : jh::test::tiny_test::session_definition<
          "test module test_flat_multimap case 1", jh::test::tiny_test::test<"basic insertion and ordering equivalence">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_flat_multimap case 1"> registration_1{};
}



namespace test {
void tiny_test_case_2() {
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

    jh::test::tiny_test::expect(static_cast<bool>((fv == sv)), "fv == sv");

}
}
template<>
struct jh::test::tiny_test::test<"duplicate key equal_range behavior">
    : jh::test::tiny_test::test_definition<"duplicate key equal_range behavior", &::test::tiny_test_case_2> {};
template<>
struct jh::test::tiny_test::session<"test module test_flat_multimap case 2">
    : jh::test::tiny_test::session_definition<
          "test module test_flat_multimap case 2", jh::test::tiny_test::test<"duplicate key equal_range behavior">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_flat_multimap case 2"> registration_2{};
}



namespace test {
void tiny_test_case_3() {
    flat_multimap<int, int> fm;
    std::multimap<int, int> sm;

    for (int i = 0; i < 10; ++i) {
        fm.insert(std::forward_as_tuple(i % 3, i));
        sm.insert({i % 3, i});
    }

    auto fc = fm.erase(1);
    auto sc = sm.erase(1);

    jh::test::tiny_test::expect(static_cast<bool>((fc == sc)), "fc == sc");
    jh::test::tiny_test::expect(static_cast<bool>((helper::dump_multimap(fm) == helper::dump_multimap(sm))), "helper::dump_multimap(fm) == helper::dump_multimap(sm)");

}
}
template<>
struct jh::test::tiny_test::test<"erase by key equivalence">
    : jh::test::tiny_test::test_definition<"erase by key equivalence", &::test::tiny_test_case_3> {};
template<>
struct jh::test::tiny_test::session<"test module test_flat_multimap case 3">
    : jh::test::tiny_test::session_definition<
          "test module test_flat_multimap case 3", jh::test::tiny_test::test<"erase by key equivalence">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_flat_multimap case 3"> registration_3{};
}



namespace test {
void tiny_test_case_4() {
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
            jh::test::tiny_test::expect(static_cast<bool>((fit == fm.end())), "fit == fm.end()");
        } else {
            jh::test::tiny_test::expect(static_cast<bool>((fit != fm.end())), "fit != fm.end()");
            jh::test::tiny_test::expect(static_cast<bool>((fit->first == sit->first)), "fit->first == sit->first");
            jh::test::tiny_test::expect(static_cast<bool>((fit->second == sit->second)), "fit->second == sit->second");
        }
    }

}
}
template<>
struct jh::test::tiny_test::test<"find behavior equivalence">
    : jh::test::tiny_test::test_definition<"find behavior equivalence", &::test::tiny_test_case_4> {};
template<>
struct jh::test::tiny_test::session<"test module test_flat_multimap case 4">
    : jh::test::tiny_test::session_definition<
          "test module test_flat_multimap case 4", jh::test::tiny_test::test<"find behavior equivalence">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_flat_multimap case 4"> registration_4{};
}




namespace test {
void tiny_test_case_5() {
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

    jh::test::tiny_test::expect(static_cast<bool>((helper::dump_multimap(fm) == helper::dump_multimap(sm))), "helper::dump_multimap(fm) == helper::dump_multimap(sm)");

}
}
template<>
struct jh::test::tiny_test::test<"range erase equivalence">
    : jh::test::tiny_test::test_definition<"range erase equivalence", &::test::tiny_test_case_5> {};
template<>
struct jh::test::tiny_test::session<"test module test_flat_multimap case 5">
    : jh::test::tiny_test::session_definition<
          "test module test_flat_multimap case 5", jh::test::tiny_test::test<"range erase equivalence">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_flat_multimap case 5"> registration_5{};
}



namespace test {
void tiny_test_case_6() {
    flat_multimap<int, int> fm;

    fm.insert(std::forward_as_tuple(1, 10));
    fm.insert(std::forward_as_tuple(2, 20));
    fm.insert(std::forward_as_tuple(3, 30));

    auto first = fm.find(2);
    auto last = fm.find(1);

    jh::test::tiny_test::expect_throw<std::logic_error>([&]() { (void)(fm.erase(first, last)); }, "throws std::logic_error: fm.erase(first, last)");

}
}
template<>
struct jh::test::tiny_test::test<"range erase rejects inverted iterators">
    : jh::test::tiny_test::test_definition<"range erase rejects inverted iterators", &::test::tiny_test_case_6> {};
template<>
struct jh::test::tiny_test::session<"test module test_flat_multimap case 6">
    : jh::test::tiny_test::session_definition<
          "test module test_flat_multimap case 6", jh::test::tiny_test::test<"range erase rejects inverted iterators">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_flat_multimap case 6"> registration_6{};
}



namespace test {
void tiny_test_case_7() {
    std::vector<std::pair<int, int>> data;
    data.reserve(100);
    for (int i = 0; i < 100; ++i)
        data.emplace_back(i % 10, i);

    flat_multimap<int, int> fm(data);
    std::multimap<int, int> sm(data.begin(), data.end());

    jh::test::tiny_test::expect(static_cast<bool>((helper::dump_multimap(fm) == helper::dump_multimap(sm))), "helper::dump_multimap(fm) == helper::dump_multimap(sm)");

}
}
template<>
struct jh::test::tiny_test::test<"bulk construction then sort equivalence">
    : jh::test::tiny_test::test_definition<"bulk construction then sort equivalence", &::test::tiny_test_case_7> {};
template<>
struct jh::test::tiny_test::session<"test module test_flat_multimap case 7">
    : jh::test::tiny_test::session_definition<
          "test module test_flat_multimap case 7", jh::test::tiny_test::test<"bulk construction then sort equivalence">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_flat_multimap case 7"> registration_7{};
}



namespace test {
void tiny_test_case_8() {
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

    jh::test::tiny_test::expect(static_cast<bool>((helper::dump_multimap(fm) == helper::dump_multimap(sm))), "helper::dump_multimap(fm) == helper::dump_multimap(sm)");

}
}
template<>
struct jh::test::tiny_test::test<"emplace basic insertion equivalence">
    : jh::test::tiny_test::test_definition<"emplace basic insertion equivalence", &::test::tiny_test_case_8> {};
template<>
struct jh::test::tiny_test::session<"test module test_flat_multimap case 8">
    : jh::test::tiny_test::session_definition<
          "test module test_flat_multimap case 8", jh::test::tiny_test::test<"emplace basic insertion equivalence">
      > {};
namespace test {
    [[maybe_unused]] const tiny_test::session<"test module test_flat_multimap case 8"> registration_8{};
}

