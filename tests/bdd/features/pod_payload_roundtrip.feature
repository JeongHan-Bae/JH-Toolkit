Feature: POD values survive binary compression

  Scenario Outline: Unsigned values survive byte conversion and Huffman compression
    Given a POD pair with values <first> and <second>
    When the POD pair is represented as bytes
    And the byte payload is compressed with canonical Huffman coding
    And the compressed payload is decompressed
    And the recovered bytes are interpreted as the original POD pair
    Then both values in the recovered POD pair match the original

    Examples:
      | first      | second     |
      | 42         | 1337       |
      | 0          | 0          |
      | 2147483648 | 4294967295 |
      | 4294967295 | 1          |
