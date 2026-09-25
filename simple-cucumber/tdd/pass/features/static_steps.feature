Feature: static step definitions

  Scenario: static Given When and Then methods can be bound
    Given the static counter starts at 4
    Given a constexpr static Given step
    When the static counter increments
    When a constexpr static When step
    Then the static counter is 5
    Then a constexpr static Then step
