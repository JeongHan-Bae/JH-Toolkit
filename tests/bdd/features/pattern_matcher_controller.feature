Feature: Three controllers share verb-specific immutable pattern matchers

  Scenario Outline: GET handlers return JSON from validated URL captures
    Given three stateless controllers registered once
    And an empty request entity
    When a GET request for <url> is dispatched
    Then the response status is 200
    And the response media type is application/json
    And the JSON response field <field> is <value>

    Examples:
      | url                                                       | field     | value                                |
      | /api/auth/status                                          | status    | ready                                |
      | /api/catalog/orders/42                                    | order_id  | 42                                   |
      | /api/users/550e8400-e29b-41d4-a716-446655440000         | user_id   | 550e8400-e29b-41d4-a716-446655440000 |
      | /api/users/550E8400-E29B-41D4-A716-446655440000         | user_id   | 550E8400-E29B-41D4-A716-446655440000 |
      | /api/users/550e8400-e29b-41d4-a716-446655440000/details | detail_id | 550e8400-e29b-41d4-a716-446655440000 |
      | /api/catalog/items/special                                | item      | special                              |
      | /api/catalog/items/other                                  | item      | other                                |

  Scenario: An exact item path wins over the string wildcard
    Given three stateless controllers registered once
    And an empty request entity
    When a GET request for /api/catalog/items/special is dispatched
    Then the response status is 200
    And the JSON response field featured is true

  Scenario: Two captures keep their URL order
    Given three stateless controllers registered once
    And an empty request entity
    When a GET request for /api/users/profile/john_doe/42 is dispatched
    Then the response status is 200
    And the JSON response field name is john_doe
    And the JSON response field number is 42

  Scenario: A JSON request entity reaches the login controller
    Given three stateless controllers registered once
    And a JSON request entity with password set to demo-secret
    When a POST request for /api/auth/login/john_doe is dispatched
    Then the response status is 200
    And the response media type is application/json
    And the JSON response field user is john_doe
    And the JSON response field authenticated is true

  Scenario: A plain text request entity reaches the catalog controller
    Given three stateless controllers registered once
    And a plain text request entity hello-from-client
    When a POST request for /api/catalog/notes/42 is dispatched
    Then the response status is 200
    And the response media type is application/json
    And the JSON response field note_id is 42
    And the JSON response field text is hello-from-client

  Scenario Outline: Invalid URLs and wrong verbs do not invoke a handler
    Given three stateless controllers registered once
    And an empty request entity
    When a <verb> request for <url> is dispatched
    Then the response status is 404
    And the response media type is application/json
    And the JSON response field error is not-found

    Examples:
      | verb | url                                                        |
      | GET  | /api/auth/login/john_doe                                   |
      | POST | /api/auth/status                                           |
      | GET  | /api/catalog/orders/not-a-number                           |
      | GET  | /api/users/550e8400-e29b-41d4-a716-44665544000G         |
      | GET  | /api/users/550e8400-e29b-41d4-a716-446655440000x        |
      | GET  | /api/users/550e8400-e29b-41d4-a716-446655440000x/details |
      | GET  | /api/users/550e8400-e29b-41d4-a716-446655440000/tail    |
      | GET  | /api/users/profile/john/doe/42                             |
      | POST | /other/auth/login/john_doe                                 |
      | GET  | /v1/catalog/items/special                                  |
      | GET  | /apix/catalog/orders/42                                    |

  Scenario: A handler rejects a plain text entity where JSON is required
    Given three stateless controllers registered once
    And a plain text request entity demo-secret
    When a POST request for /api/auth/login/john_doe is dispatched
    Then the response status is 415
    And the JSON response field error is unsupported-media-type

  Scenario: A handler rejects a JSON entity where plain text is required
    Given three stateless controllers registered once
    And a JSON request entity with note set to hello
    When a POST request for /api/catalog/notes/42 is dispatched
    Then the response status is 415
    And the JSON response field error is unsupported-media-type

  Scenario: Login rejects JSON without the required password field
    Given three stateless controllers registered once
    And a JSON request entity with other set to value
    When a POST request for /api/auth/login/john_doe is dispatched
    Then the response status is 400
    And the JSON response field error is missing-password

  Scenario: A handler rejects the wrong number of captures
    Given three stateless controllers registered once
    And an empty request entity
    When the login handler receives no captures
    Then the response status is 400
    And the JSON response field error is capture-count
