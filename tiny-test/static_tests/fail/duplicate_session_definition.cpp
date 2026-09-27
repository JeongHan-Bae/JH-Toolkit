#include "jh/test/tiny_test/tiny_test.hpp"

template<>
struct jh::test::tiny_test::session<"duplicate session">
    : jh::test::tiny_test::session_definition<"duplicate session"> {};

template<>
struct jh::test::tiny_test::session<"duplicate session">
    : jh::test::tiny_test::session_definition<"duplicate session"> {};

