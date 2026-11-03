Feature: String-backed value conversion preserves live views

  Scenario: A string view borrows its live source text
    Given a live text value
    When the text is deserialized as a string view
    Then the view refers to the live source text

  Scenario: A view-containing value round trips through a string
    Given a JSON text value
    When the JSON view-like value is serialized
    And it is reconstructed from the serialized string
    Then the reconstructed view refers to the serialized string

  Scenario: Stream extraction consumes an adapter while preserving source text
    Given numeric text 42
    When the numeric text is deserialized through a stream
    Then the parsed value is 42 and the source text is unchanged
