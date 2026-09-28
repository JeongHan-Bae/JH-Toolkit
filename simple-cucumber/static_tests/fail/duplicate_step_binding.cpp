#include "jh/test/cucumber/cucumber.hpp"

namespace {
    struct steps {
        void run() {}
    };

    using definition = jh::test::cucumber::StepDefinition<
        steps,
        jh::test::cucumber::Given<"the same step", &steps::run>,
        jh::test::cucumber::Given<"the same step", &steps::run>
    >;

    definition invalid_definition;
}
