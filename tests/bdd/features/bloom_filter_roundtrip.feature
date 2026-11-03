Feature: String-view Bloom filter membership survives a fixed binary record

  Scenario Outline: A record restores membership across storage modes
    Given an <source> Bloom filter for string-view keys with 65 bits
    And an <target> destination containing the marker key
    When the pear and plum keys are inserted
    And the filter is saved as a binary record
    And the binary record is restored into the destination
    Then the pear and plum keys are possibly present
    And the kiwi and marker keys are definitely absent
    And the binary record has its compile-time length

    Examples:
      | source    | target    |
      | inline    | inline    |
      | inline    | allocated |
      | allocated | inline    |
      | allocated | allocated |

  Scenario: Empty and word-boundary string views survive restoration
    Given an inline Bloom filter for string-view keys with 65 bits
    And an allocated destination containing the marker key
    When the empty and 64-byte keys are inserted
    And the filter is saved as a binary record
    And the binary record is restored into the destination
    Then the empty and 64-byte keys are possibly present
    And the marker key is definitely absent

  Scenario: A damaged signature leaves the destination intact
    Given an inline Bloom filter for string-view keys with 65 bits
    And an allocated destination containing the marker key
    When the pear and plum keys are inserted
    And the filter is saved as a binary record
    And the signature byte is changed
    And restoration is attempted
    Then the damaged record is rejected
    And the marker key remains possibly present
    And the pear key remains definitely absent
