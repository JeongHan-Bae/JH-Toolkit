#include "jh/test/tiny_test/tiny_test.hpp"

extern int tiny_test_ambiguous_runs;
int tiny_test_ambiguous_runs{};

int main(const int argc, char** argv) {
    const int status = jh::test::tiny_test::tiny_main(0, nullptr);
    const auto& errors = jh::test::tiny_test::detail::registry().errors;
    bool found_expected_ambiguity = false;
    const std::string_view expected_kind = argc > 1 && argv != nullptr && argv[1] != nullptr
                                               ? argv[1]
                                               : std::string_view{};
    for (const auto& error : errors) {
        const bool duplicate_test = expected_kind == "test"
                                    && error.find("'shared test'") != std::string::npos
                                    && error.find("test conflicts with an existing test")
                                           != std::string::npos;
        const bool duplicate_session = expected_kind == "session"
                                       && error.find("'shared session'") != std::string::npos
                                       && error.find("session conflicts with an existing session")
                                              != std::string::npos;
        found_expected_ambiguity = found_expected_ambiguity || duplicate_test || duplicate_session;
    }
    return status != 0 && tiny_test_ambiguous_runs == 0 && found_expected_ambiguity ? 0 : 1;
}
