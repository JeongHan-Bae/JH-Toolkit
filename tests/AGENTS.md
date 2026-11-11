# Test Conventions

Place compile-time contracts and compile-pass/fail checks in `tests/static/`.
Use `tests/tdd/unit/` and `tests/tdd/composed/` for runtime behavior, with one
tiny-test executable and CTest entry per test module. Use `tests/bdd/` for
caller-visible behavior expressed as Gherkin scenarios. Follow
`tests/bdd/AGENTS.md` for feature and step organization.

## Concurrent behavior

Test a specific interleaving with explicit synchronization such as a latch,
barrier, condition variable, or a controllable callback. Arrange the threads
so the contested operation is known to be in progress before performing the
conflicting operation, then join every thread before asserting the outcome.
Prefer one reproducible transition and a precise invariant over many iterations
that merely increase the chance of a race. Do not use random scheduling,
sleep durations, or repeated attempts as the primary evidence for a concurrency
correctness claim. Keep timeouts only as bounds for operations whose contract
actually includes time or to prevent a broken test from hanging indefinitely.
Stress runs and third-party detectors such as ThreadSanitizer may reveal risks,
but their diagnostics and exit codes do not count as a red or green behavior
test. For a defined-behavior concurrency defect, start with a separate red
`.cpp` that makes a tiny-test assertion fail on the original implementation
under a controlled interleaving. Run the same assertion after the fix and
record both outcomes.

For an internal fix, temporary fault injection may establish the red-green
behavior, but both outcomes must be shown by test-framework assertions.
Undefined behavior does not require a red-green demonstration; identify the
relevant ISO C++ requirement, the exact operation that violates it, and the
limits of runtime evidence.
After the green run, move a test that uses ordinary behavior into its unit or
composed module and delete the standalone `.cpp`. If the test
depends entirely on injected fixtures or production-source hooks, remove the
test and hooks before delivery, then report the diagnostic result and the
absence of permanent regression coverage. A public API change that improves
control, such as a configurable RW mutex, may justify a retained test; confirm
the intended API change with the developer unless the change is already
authorized.

Pool tests that assert cross-thread synchronization must inject a known RW
mutex and verify the relevant lock path. Do not rely on the default
`std::shared_mutex` to establish Windows runtime correctness. A test RW mutex
may use a `std::mutex` entrance gate and a reader count, following the IPC
shared mutex strategy. Compile-time default-API checks and single-threaded
behavior tests may use the default mutex.

## TDD

Use tiny-test's `expect`, `expect_not`, and typed `expect_throw<Exception>`
for assertions. Pass an operation that should throw to `expect_throw`. Call
an operation that should complete normally directly; the tiny-test runner
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
