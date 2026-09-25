Feature: typed steps and scenario state

  Scenario Outline: increment a counter with typed values
    Given a counter named <name> has <value> messages
    And the counter is named <name>
    When one message is added
    And one more message is added
    Then the counter named <name> has <result> messages
    And the counter has a positive value

    Examples:
      | name  | value | result |
      | first | 41    | 43     |
      | next  | 6     | 8      |

  Scenario: each scenario receives a fresh step object
    Then the counter is 0
