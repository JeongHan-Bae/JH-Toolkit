Feature: step definitions with injected services

  Scenario: create and sign in a user
    Given I am a user
    Then I can sign in

  Scenario: create another user with a fresh Steps object
    Given I am a user
    Then I can sign in
