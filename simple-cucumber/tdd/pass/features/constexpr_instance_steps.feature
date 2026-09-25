Feature: constexpr instance step definitions

  Scenario: constexpr non-static Given and When methods can be bound
    Given the constexpr instance counter starts at 10
    When the constexpr instance counter increments
    Then the constexpr instance counter is 11
    Then a constexpr instance Then step
