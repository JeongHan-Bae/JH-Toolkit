Feature: URI components preserve their text

  Scenario Outline: Common URI text uses its expected percent encoding
    Given a URI component containing <text>
    When it is percent encoded
    And the encoded component is decoded
    Then the encoded URI component is <encoded>
    And the recovered URI component matches the original

    Examples:
      | text          | encoded               |
      | abc           | abc                   |
      | Hello World   | Hello%20World         |
      | Hello+World   | Hello%2BWorld         |
      | a/b?c=d&e=f   | a%2Fb%3Fc%3Dd%26e%3Df |
      | 100%          | 100%25                |
      | #fragment     | %23fragment           |
      | A B C         | A%20B%20C             |

  Scenario: An empty URI component remains empty
    Given an empty URI component
    When it is percent encoded
    And the encoded component is decoded
    Then an empty input produces no encoded URI text
    And the recovered URI component matches the original

  Scenario: Percent-encoded text decodes across hex letter cases
    Given the following encoded URI component pairs
      | encoded       | expected      |
      | Hello%20World | Hello World   |
      | a%2fb          | a/b           |
      | a%2Fb          | a/b           |
      | %23fragment    | #fragment     |
    When each encoded URI component is decoded
    Then each decoded component matches its expected text

  Scenario: Malformed percent encodings are rejected
    Given the following malformed percent-encoded values
      | encoded |
      | %       |
      | %A      |
      | abc%    |
      | %GG     |
      | %1G     |
      | %G1     |
    When each malformed URI component is decoded
    Then each malformed URI component is rejected

  Scenario: Raw control characters in encoded URI text are rejected
    Given encoded URI text containing a raw control character
    When each malformed URI component is decoded
    Then each malformed URI component is rejected
