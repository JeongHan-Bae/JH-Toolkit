# BDD Step Conventions

Describe behavior promised to callers as concrete inputs and observable results, including special and rejected inputs
when they are part of the API contract. Use Scenario Outlines or DataTables for related cases, and give each meaningful
operation in a roundtrip its own `When`/`And` step.

Keep each Gherkin feature paired with its own `steps/<feature>_steps.hpp` file. Define that feature's step class and
single `StepDefinition` in the header, with step bodies inline. Do not add `.cpp` step files or combine unrelated
features in one step header. Keep implementation details and load/stress testing in their appropriate suites. The
runner loads each feature with its matching definition.
