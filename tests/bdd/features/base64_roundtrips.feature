Feature: Base64 encodings preserve byte sequences

  Scenario Outline: Common binary values have the expected Base64 representation
    Given a byte sequence represented by hexadecimal <hex>
    When it is encoded with <format>
    And the encoded text is decoded
    Then the encoded Base64 string is <encoded>
    And the recovered byte sequence matches the original

    Examples:
      | hex                              | format                    | encoded                 |
      | 48656C6C6F                       | Base64                    | SGVsbG8=                |
      | 00FF1020                         | Base64URL without padding | AP8QIA                  |
      | 00FF1020                         | Base64URL with padding    | AP8QIA==                |
      | 000102030405060708090A0B0C0D0E0F | Base64                    | AAECAwQFBgcICQoLDA0ODw== |
      | 000102030405060708090A0B0C0D0E0F | Base64URL without padding | AAECAwQFBgcICQoLDA0ODw  |
      | FFFFFFFFFFFFFFFF                 | Base64                    | //////////8=            |
      | FFFFFFFFFFFFFFFF                 | Base64URL with padding    | __________8=            |
      | FFFFFFFFFFFFFFFF                 | Base64URL without padding | __________8             |
      | 00FF00FF00FF00FF                 | Base64                    | AP8A/wD/AP8=            |
      | 00FF00FF00FF00FF                 | Base64URL with padding    | AP8A_wD_AP8=            |
      | 00FF00FF00FF00FF                 | Base64URL without padding | AP8A_wD_AP8             |

  Scenario: An empty byte sequence has an empty representation
    Given an empty byte sequence
    When it is encoded with Base64
    And the encoded text is decoded
    Then encoding an empty sequence produces no Base64 text
    And the recovered byte sequence matches the original

  Scenario: Malformed Base64 inputs are rejected
    Given the following malformed Base64 texts
      | encoded |
      | A       |
      | ABC     |
      | AA$B==  |
      | A@BC    |
      | AAAA=== |
      | AAAAA=  |
    When each text is decoded
    Then each malformed input is rejected

  Scenario: A NUL byte inside Base64 text is rejected
    Given Base64 text containing an embedded NUL byte
    When each text is decoded
    Then each malformed input is rejected
