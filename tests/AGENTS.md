# Test Conventions

Place compile-time contracts and compile-pass/fail checks in `tests/static/`.
Use `tests/tdd/unit/` and `tests/tdd/composed/` for runtime behavior, with one
TinyTest executable and CTest entry per test module. Use `tests/bdd/` for
caller-visible behavior expressed as Gherkin scenarios. Follow
`tests/bdd/AGENTS.md` for feature and step organization.

## TDD

Use TinyTest's `expect`, `expect_not`, and typed `expect_throw<Exception>`
for assertions. Pass an operation that should throw to `expect_throw`. Call
an operation that should complete normally directly; the TinyTest runner
records an exception that reaches its outer boundary.

A local `try/catch` can capture an exception's error code or message. Rethrow
inside `expect_throw<Exception>` to check its type, then assert the captured
payload separately. Teardown code can catch exceptions to finish releasing
test resources.

## BDD

Use `Given` for setup, `When` or `And` for an operation, and `Then` with
`StepContext::expect` for its observable result. `run` and `run_file` pass
their arguments as lvalues to a new step object for each scenario. A temporary
argument remains alive for the synchronous run. Each step object owns its
scenario state and releases its resources when the scenario scope ends.
An injected service held by reference can retain state across scenarios in
the same run. Each step receives its own `StepContext`; assertion failures
enter the scenario result and end that scenario's step sequence.

For a rejected input, a `When` step can catch the expected exception type and
record the outcome in its scenario object. A separate `Then` step checks the
outcome with `StepContext::expect`. An exception with another type propagates
through the runner and invokes normal RAII cleanup. When a scenario repeats
an operation, update its outcome state for each attempt.
