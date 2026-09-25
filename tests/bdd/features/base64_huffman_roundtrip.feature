Feature: Base64 text can be recovered after Huffman compression

  Scenario Outline: Encoded text can be compressed, recovered and decoded
    Given a byte sequence represented by hexadecimal <hex>
    When it is encoded with Base64
    And the Base64 text is compressed with canonical Huffman coding
    And the compressed data is decompressed
    And the recovered text is decoded from Base64
    Then the recovered byte sequence matches the original

    Examples:
      | hex                              |
      | 004A48FF1020                     |
      | 48656C6C6F                       |
      | 00                               |
      | FF                               |
      | 000102030405060708090A0B0C0D0E0F |
