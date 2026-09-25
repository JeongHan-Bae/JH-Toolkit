#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "jh/test/cucumber/cucumber.hpp"

namespace test::cucumber {
    using namespace jh::test::cucumber;
}

namespace test::fail {
    class ConversionSteps final {
    public:
        inline static std::size_t invocation_count{};

        static void set_integer(const std::int64_t) {
            ++invocation_count;
        }
    };

    using ConversionDefinition = cucumber::StepDefinition<
        ConversionSteps,
        cucumber::Given<"the integer is <int>", &ConversionSteps::set_integer>
    >;

    class UndefinedSteps final {
    public:
        void bind_counter(const std::uint64_t) {}
    };

    using UndefinedDefinition = cucumber::StepDefinition<
        UndefinedSteps,
        cucumber::Given<"a counter has <uint> messages", &UndefinedSteps::bind_counter>
    >;

    class AmbiguousSteps final {
    public:
        inline static std::size_t invocation_count{};

        void description_ending_in_large(const std::string_view) {
            ++invocation_count;
        }

        void description_with_large(const std::string_view) {
            ++invocation_count;
        }
    };

    using AmbiguousDefinition = cucumber::StepDefinition<
        AmbiguousSteps,
        cucumber::Given<"<string> is large", &AmbiguousSteps::description_ending_in_large>,
        cucumber::Given<"<string> large", &AmbiguousSteps::description_with_large>
    >;

    class AmbiguousScalarSteps final {
    public:
        inline static std::size_t invocation_count{};

        void string_is_cool(const std::string_view) { ++invocation_count; }
        void integer_is_cool(const std::int64_t) { ++invocation_count; }
        void unsigned_is_cool(const std::uint64_t) { ++invocation_count; }
    };

    using AmbiguousScalarDefinition = cucumber::StepDefinition<
        AmbiguousScalarSteps,
        cucumber::Given<"<string> is cool", &AmbiguousScalarSteps::string_is_cool>,
        cucumber::Given<"<int> is cool", &AmbiguousScalarSteps::integer_is_cool>,
        cucumber::Given<"<uint> is cool", &AmbiguousScalarSteps::unsigned_is_cool>
    >;

    class DuplicateBindingSteps final {
    public:
        void first() {}
        void second() {}
    };

    using DuplicateGivenFirst = cucumber::Given<"the same step", &DuplicateBindingSteps::first>;
    using DuplicateGivenSecond = cucumber::Given<"the same step", &DuplicateBindingSteps::second>;
    using SameTextWhen = cucumber::When<"the same step", &DuplicateBindingSteps::second>;

    static_assert(!cucumber::detail::unique_step_bindings_v<
        DuplicateGivenFirst,
        DuplicateGivenSecond
    >);
    static_assert(cucumber::detail::unique_step_bindings_v<
        DuplicateGivenFirst,
        SameTextWhen
    >);

    class TableAttachmentSteps final {
    public:
        inline static std::size_t without_table_calls{};
        inline static std::size_t with_table_calls{};

        void without_table() { ++without_table_calls; }
        void with_table(const cucumber::DataTable&) { ++with_table_calls; }
        void with_mutable_table(cucumber::DataTable&) {}
    };

    using ConstTableBinding = cucumber::Given<
        "the following configuration",
        &TableAttachmentSteps::with_table
    >;
    using MutableTableBinding = cucumber::Given<
        "the following configuration",
        &TableAttachmentSteps::with_mutable_table
    >;

    static_assert(cucumber::detail::binding_signature_valid<
        TableAttachmentSteps,
        ConstTableBinding
    >());
    static_assert(!cucumber::detail::binding_signature_valid<
        TableAttachmentSteps,
        MutableTableBinding
    >());

    using UnexpectedDataTableDefinition = cucumber::StepDefinition<
        TableAttachmentSteps,
        cucumber::Given<"the following configuration", &TableAttachmentSteps::without_table>
    >;
    using RequiredDataTableDefinition = cucumber::StepDefinition<
        TableAttachmentSteps,
        ConstTableBinding
    >;
}
