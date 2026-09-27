#include "jh/test/tiny_test/tiny_test.hpp"

namespace test::tiny_test {
    using namespace jh::test::tiny_test;
}

namespace test {
    void first_duplicate_body() { tiny_test::expect(true); }
    void second_duplicate_body() { tiny_test::expect(false); }
}

template<>
struct jh::test::tiny_test::test<"duplicate test">
    : jh::test::tiny_test::test_definition<"duplicate test", &::test::first_duplicate_body> {};

template<>
struct jh::test::tiny_test::test<"duplicate test">
    : jh::test::tiny_test::test_definition<"duplicate test", &::test::second_duplicate_body> {};
