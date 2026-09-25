Feature: Boolean values survive runtime storage conversion

  Scenario Outline: Boolean vectors survive runtime-sized storage conversion
    Given boolean values <values>
    When they are moved into a runtime-sized bit sequence
    And the sequence is moved back to a boolean vector
    Then the original boolean values are restored

    Examples:
      | values       |
      | 1            |
      | 0            |
      | 10110        |
      | 00000000     |
      | 11111111     |
      | 010101010101 |

  Scenario: An empty boolean vector remains empty
    Given an empty boolean vector
    When they are moved into a runtime-sized bit sequence
    And the sequence is moved back to a boolean vector
    Then the original boolean values are restored

  Scenario: Boolean values across a storage-word boundary are preserved
    Given 65 alternating boolean values
    When they are moved into a runtime-sized bit sequence
    And the sequence is moved back to a boolean vector
    Then the original boolean values are restored
