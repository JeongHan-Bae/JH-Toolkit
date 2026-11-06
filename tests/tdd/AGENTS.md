# TDD Test Layout

Follow `tests/AGENTS.md` for test behavior and red-green requirements.

Place reusable test fixtures according to their consumers:

- Fixtures used only by unit tests: `tests/tdd/unit/support/`.
- Fixtures used only by composed tests: `tests/tdd/composed/support/`.
- Fixtures shared by unit and composed tests: `tests/tdd/support/`.

Keep a fixture local to its test source when no other module uses it.
