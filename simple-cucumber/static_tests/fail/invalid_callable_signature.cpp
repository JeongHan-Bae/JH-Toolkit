#include "jh/test/cucumber/cucumber.hpp"

namespace {
    struct steps {
        bool run(std::int64_t) { return true; }
    };

    using definition = jh::test::cucumber::StepDefinition<
        steps,
        jh::test::cucumber::Given<"value <int>", &steps::run>
    >;

    definition invalid_definition;
}
