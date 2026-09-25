Feature: Safe URI text operations validate text boundaries

  Scenario Outline: Legal UTF-8 text can be safely encoded and decoded
    Given legal UTF-8 text <text>
    When it is encoded with text validation
    And the encoded text is decoded with text validation
    Then the safe encoded URI text is <encoded>
    And the recovered text matches the original

    Examples:
      | text        | encoded                       |
      | abc         | abc                           |
      | Hello World | Hello%20World                 |
      | café        | caf%C3%A9                     |
      | 你好        | %E4%BD%A0%E5%A5%BD            |

  Scenario Outline: Safe encoding rejects control bytes and invalid UTF-8
    Given text bytes represented by hexadecimal <hex>
    When the candidate is safely encoded
    Then safe encoding rejects the candidate

    Examples:
      | hex    |
      | 610162 |
      | 410042 |
      | C328   |

  Scenario Outline: Safe decoding rejects control bytes and invalid UTF-8
    Given percent-encoded text <encoded>
    When it is decoded as safe URI text
    Then safe decoding rejects the result

    Examples:
      | encoded |
      | %01     |
      | %00     |
      | %C3%28  |
