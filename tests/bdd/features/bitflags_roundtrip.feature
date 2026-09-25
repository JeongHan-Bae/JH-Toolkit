Feature: Flag state survives saving and restoring

  Scenario Outline: Enabled flag positions are preserved
    Given a 16-bit flag set with enabled bits <indices>
    When its state is saved to bytes
    And the state is restored from those bytes
    Then the restored flag set contains exactly those bits

    Examples:
      | indices                   |
      | 0                         |
      | 15                        |
      | 7,8                       |
      | 0,7,8,15                  |
      | 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 |

  Scenario: A flag set with no enabled positions remains empty
    Given a 16-bit flag set with no enabled bits
    When its state is saved to bytes
    And the state is restored from those bytes
    Then the restored flag set contains no enabled bits
