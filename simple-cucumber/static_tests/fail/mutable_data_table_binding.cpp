#include "jh/test/cucumber/cucumber.hpp"

namespace {
    struct steps {
        void run(jh::test::cucumber::DataTable&) {}
    };

    using definition = jh::test::cucumber::StepDefinition<
        steps,
        jh::test::cucumber::Given<"the table", &steps::run>
    >;

    definition invalid_definition;
}
