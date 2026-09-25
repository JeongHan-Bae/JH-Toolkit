Feature: Huffman compression preserves binary input

  Scenario Outline: Binary sequences can be compressed and recovered
    Given a byte sequence represented by hexadecimal <hex>
    When it is compressed with canonical Huffman coding
    And the compressed Huffman stream is decompressed
    Then the recovered byte sequence matches the original

    Examples:
      | hex                              |
      | 004A48FF                         |
      | 00                               |
      | 4141414141414141                 |
      | FF00807F                          |
      | 000102030405060708090A0B0C0D0E0F |

  Scenario: An empty byte sequence can be compressed and recovered
    Given an empty byte sequence
    When it is compressed with canonical Huffman coding
    And the compressed Huffman stream is decompressed
    Then the recovered byte sequence matches the original

  Scenario: A stream from another codec signature is rejected
    Given a stream compressed for another application
    When the current application tries to decompress that stream
    Then the incompatible stream is rejected
