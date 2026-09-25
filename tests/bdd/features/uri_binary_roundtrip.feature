Feature: URI percent-encoding preserves byte payloads

  Scenario Outline: Percent encoding preserves arbitrary payload bytes
    Given a byte payload represented by hexadecimal <hex>
    When the payload is percent-encoded
    And the encoded URI text is decoded
    Then the encoded URI text is <encoded>
    And the recovered byte payload matches the original

    Examples:
      | hex    | encoded       |
      | 410042 | A%00B         |
      | 00FF10 | %00%FF%10     |
      | 486921 | Hi%21         |
