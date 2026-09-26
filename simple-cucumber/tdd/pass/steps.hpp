/**
 * @file simple-cucumber/tdd/pass/steps.hpp
 * @brief Passing test step definitions for simple-cucumber.
 */

#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include "jh/test/cucumber/cucumber.hpp"

namespace test::cucumber {
    using namespace jh::test::cucumber;
}

namespace test::pass {
    class CounterSteps final {
        std::string_view name_;
        std::uint64_t value_{};

    public:
        void start(const std::string_view name, const std::uint64_t value) noexcept(false) {
            name_ = name;
            value_ = value;
        }

        void increment() noexcept {
            ++value_;
        }

        void expect_name(const std::string_view name, cucumber::StepContext& context) {
            context.expect(name_ == name, "Given And step should inherit Given kind");
        }

        void expect_counter(
            const std::string_view name,
            const std::uint64_t expected,
            cucumber::StepContext& context
        ) noexcept(false) {
            context.expect(name_ == name, "counter name differs");
            context.expect(value_ == expected, "counter value differs");
        }

        void expect_value(
            const std::uint64_t expected,
            cucumber::StepContext& context
        ) const noexcept {
            context.expect(value_ == expected, "counter was not reset between scenarios");
        }

        void expect_positive(cucumber::StepContext& context) const {
            context.expect(value_ > 0, "Then And step should inherit Then kind");
        }
    };

    using CounterDefinition = cucumber::StepDefinition<
        CounterSteps,
        cucumber::Given<"a counter named <string> has <uint> messages", &CounterSteps::start>,
        cucumber::Given<"the counter is named <string>", &CounterSteps::expect_name>,
        cucumber::When<"one message is added", &CounterSteps::increment>,
        cucumber::When<"one more message is added", &CounterSteps::increment>,
        cucumber::Then<"the counter named <string> has <uint> messages", &CounterSteps::expect_counter>,
        cucumber::Then<"the counter is <uint>", &CounterSteps::expect_value>,
        cucumber::Then<"the counter has a positive value", &CounterSteps::expect_positive>
    >;

    class SignedUnsignedSteps final {
    public:
        inline static std::size_t signed_calls{};
        inline static std::size_t unsigned_calls{};
        inline static std::int64_t signed_value{};

        static void receive_signed(const std::int64_t value) {
            ++signed_calls;
            signed_value = value;
        }

        static void receive_unsigned(const std::uint64_t) {
            ++unsigned_calls;
        }
    };

    using SignedUnsignedDefinition = cucumber::StepDefinition<
        SignedUnsignedSteps,
        cucumber::Given<"<int> is cool", &SignedUnsignedSteps::receive_signed>,
        cucumber::Given<"<uint> is cool", &SignedUnsignedSteps::receive_unsigned>
    >;

    class ExponentDoubleSteps final {
    public:
        static void check(const double value, cucumber::StepContext& context) {
            context.expect(value == 2e6, "exponent-form double differs");
        }
    };

    using ExponentDoubleDefinition = cucumber::StepDefinition<
        ExponentDoubleSteps,
        cucumber::Given<"<double> is cool", &ExponentDoubleSteps::check>
    >;

    class UserService {
    public:
        virtual ~UserService() = default;
        virtual void add_user() = 0;
        [[nodiscard]] virtual bool has_user() const = 0;
    };

    class InMemoryUserService final : public UserService {
        bool user_exists_{};

    public:
        void add_user() override { user_exists_ = true; }
        [[nodiscard]] bool has_user() const override { return user_exists_; }
    };

    class DatabaseUserService final : public UserService {
        std::uint64_t user_rows_{};
        std::string& lifecycle_;

    public:
        explicit DatabaseUserService(std::string& lifecycle): lifecycle_(lifecycle) {
            lifecycle_ += "connect to database\n";
        }

        ~DatabaseUserService() override {
            lifecycle_ += "disconnect from database\n";
        }

        void add_user() override { ++user_rows_; }
        [[nodiscard]] bool has_user() const override { return user_rows_ != 0; }
    };

    class UserSteps final {
        UserService& service_;

    public:
        explicit UserSteps(UserService& service): service_(service) {}

        void i_am_a_user() { service_.add_user(); }

        void i_can_sign_in(cucumber::StepContext& context) const {
            context.expect(service_.has_user(), "user service should contain the created user");
        }
    };

    using InMemoryUserDefinition = cucumber::StepDefinition<
        UserSteps,
        cucumber::Given<"I am a user", &UserSteps::i_am_a_user>,
        cucumber::Then<"I can sign in", &UserSteps::i_can_sign_in>
    >;

    using DatabaseUserDefinition = cucumber::StepDefinition<
        UserSteps,
        cucumber::Given<"I am a user", &UserSteps::i_am_a_user>,
        cucumber::Then<"I can sign in", &UserSteps::i_can_sign_in>
    >;

    class ConstSteps final {
        mutable bool given_called_{};
        mutable bool when_called_{};

    public:
        constexpr void given_const() const noexcept {
            given_called_ = true;
        }

        constexpr void when_const() const noexcept(false) {
            when_called_ = true;
        }

        void then_const(cucumber::StepContext& context) const noexcept {
            context.expect(given_called_, "const Given step should execute");
            context.expect(when_called_, "const When step should execute");
        }

        constexpr void then_constexpr() const noexcept(false) {}
    };

    using ConstDefinition = cucumber::StepDefinition<
        ConstSteps,
        cucumber::Given<"a const Given step", &ConstSteps::given_const>,
        cucumber::When<"a const When step", &ConstSteps::when_const>,
        cucumber::Then<"a const Then step", &ConstSteps::then_const>,
        cucumber::Then<"a constexpr const Then step", &ConstSteps::then_constexpr>
    >;

    class StaticSteps final {
        inline static std::uint64_t value_{};

    public:
        static void set_value(const std::uint64_t value) noexcept(false) {
            value_ = value;
        }

        static constexpr void constexpr_given() noexcept {}

        static void increment() noexcept {
            ++value_;
        }

        static constexpr void constexpr_when() noexcept(false) {}

        static void expect_value(
            const std::uint64_t expected,
            cucumber::StepContext& context
        ) noexcept(false) {
            context.expect(value_ == expected, "static counter value differs");
        }

        static constexpr void constexpr_then() noexcept {}
    };

    using StaticDefinition = cucumber::StepDefinition<
        StaticSteps,
        cucumber::Given<"the static counter starts at <uint>", &StaticSteps::set_value>,
        cucumber::Given<"a constexpr static Given step", &StaticSteps::constexpr_given>,
        cucumber::When<"the static counter increments", &StaticSteps::increment>,
        cucumber::When<"a constexpr static When step", &StaticSteps::constexpr_when>,
        cucumber::Then<"the static counter is <uint>", &StaticSteps::expect_value>,
        cucumber::Then<"a constexpr static Then step", &StaticSteps::constexpr_then>
    >;

    class InstanceConstexprSteps final {
        std::uint64_t value_{};

    public:
        constexpr void seed(const std::uint64_t value) noexcept(false) {
            value_ = value;
        }

        constexpr void increment() noexcept {
            ++value_;
        }

        void expect_value(
            const std::uint64_t expected,
            cucumber::StepContext& context
        ) const noexcept(false) {
            context.expect(value_ == expected, "constexpr instance counter differs");
        }

        constexpr void constexpr_then() noexcept(false) {}
    };

    using InstanceConstexprDefinition = cucumber::StepDefinition<
        InstanceConstexprSteps,
        cucumber::Given<"the constexpr instance counter starts at <uint>", &InstanceConstexprSteps::seed>,
        cucumber::When<"the constexpr instance counter increments", &InstanceConstexprSteps::increment>,
        cucumber::Then<"the constexpr instance counter is <uint>", &InstanceConstexprSteps::expect_value>,
        cucumber::Then<"a constexpr instance Then step", &InstanceConstexprSteps::constexpr_then>
    >;

    class DataTableSteps final {
    public:
        static void check_configuration(
            const cucumber::DataTable& table,
            cucumber::StepContext& context
        ) {
            const auto rows = table.rows();
            if (!context.expect(rows.size() == 2, "expected two configuration rows")) return;

            const auto locale_key = rows[0].at("key");
            const auto locale_value = rows[0].at("value");
            const auto attempts_key = rows[1].at("key");
            const auto attempts_text = rows[1].at("value");
            if (!context.expect(
                    locale_key && locale_value && attempts_key && attempts_text,
                    "configuration columns are missing")) {
                return;
            }

            context.expect(*locale_key == "locale", "first key should be locale");
            const auto saved_locale = cucumber::asString(*locale_value);
            context.expect(saved_locale == "zh-CN", "locale should be preserved");
            context.expect(*attempts_key == "attempts", "second key should be attempts");

            const auto attempts = cucumber::asUint(*attempts_text);
            context.expect(attempts.has_value(), "attempts should parse as an unsigned integer");
            if (attempts) context.expect(*attempts == 3, "attempt count differs");
        }
    };

    using DataTableDefinition = cucumber::StepDefinition<
        DataTableSteps,
        cucumber::Given<"the following configuration", &DataTableSteps::check_configuration>
    >;
}
