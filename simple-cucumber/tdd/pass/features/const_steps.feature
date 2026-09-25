Feature: const step definitions

  Scenario: const Given When and Then methods can be bound
    Given a const Given step
    When a const When step
    Then a const Then step
    Then a constexpr const Then step
