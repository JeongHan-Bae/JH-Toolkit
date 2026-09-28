#include "jh/test/cucumber/cucumber.hpp"

namespace {
    struct steps {
        void run(std::int64_t, std::uint64_t) {}
    };

    using definition = jh::test::cucumber::StepDefinition<
        steps,
        jh::test::cucumber::Given<"adjacent <int><uint>", &steps::run>
    >;

    definition invalid_definition;
}
