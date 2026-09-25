Feature: Indexed updates remain visible in the source sequence

  Scenario Outline: Changes made through an indexed view are visible on a later traversal
    Given a mutable integer sequence containing <values>
    When each value is set to ten times its displayed position starting at 100
    Then a later traversal shows the updated values at those positions

    Examples:
      | values      |
      | 4,1,7,2,9   |
      | -3,0,5      |
      | 42          |

  Scenario: Indexed updates also work with runtime-sized storage
    Given a runtime-sized integer sequence containing 8,3,1,6
    When each value is set to ten times its displayed position starting at 100
    Then a later traversal shows the updated values at those positions
