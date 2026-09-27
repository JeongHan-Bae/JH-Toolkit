#include "jh/test/cucumber/cucumber.hpp"

namespace {
    using namespace jh::test::cucumber;

    constexpr auto parsed_expression = parseExpression<
        "name <string> signed <int> unsigned <uint> real <double> flag <bool>"
    >();
    static_assert(parsed_expression.valid);
    static_assert(parsed_expression.parameter_count == 5);

    constexpr auto literal_expression = parseExpression<"hello">();
    static_assert(literal_expression.valid);
    static_assert(literal_expression.parameter_count == 0);

    class DuplicateBindingSteps final {
    public:
        void first() {}
        void second() {}
    };

    using DuplicateGivenFirst = Given<"the same step", &DuplicateBindingSteps::first>;
    using DuplicateGivenSecond = Given<"the same step", &DuplicateBindingSteps::second>;
    using SameTextWhen = When<"the same step", &DuplicateBindingSteps::second>;

    static_assert(!detail::unique_step_bindings_v<
        DuplicateGivenFirst,
        DuplicateGivenSecond
    >);
    static_assert(detail::unique_step_bindings_v<DuplicateGivenFirst, SameTextWhen>);

    class TableAttachmentSteps final {
    public:
        void with_table(const DataTable&) {}
        void with_mutable_table(DataTable&) {}
    };

    using ConstTableBinding = Given<
        "the following configuration",
        &TableAttachmentSteps::with_table
    >;
    using MutableTableBinding = Given<
        "the following configuration",
        &TableAttachmentSteps::with_mutable_table
    >;

    static_assert(detail::binding_signature_valid<
        TableAttachmentSteps,
        ConstTableBinding
    >());
    static_assert(!detail::binding_signature_valid<
        TableAttachmentSteps,
        MutableTableBinding
    >());
}
