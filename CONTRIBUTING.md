# Contributing to JH-Toolkit

Thank you for contributing. Keep changes focused, practical, and compatible with
existing APIs within the same major and minor version series. Document any
breaking change and its migration path.

## Project names

Use the following names consistently in documentation and user-facing text.
When the projects appear in parallel, use the same naming category for each.

| Project             | Internal short name | Public name        |
|---------------------|---------------------|--------------------|
| Core library        | `jh-toolkit`        | JH-Toolkit         |
| TDD framework       | `tiny-test`         | JH-Tiny-Test       |
| BDD framework       | `simple-cucumber`   | JH-Simple-Cucumber |

The CMake build targets have their own names: `jh-toolkit` and
`jh::jh-toolkit`; `jh-tiny-test` and `jh::test::jh-tiny-test`;
`jh-simple-cucumber` and `jh::test::jh-simple-cucumber`. Use those exact names
when referring to the targets.

## Branches

The project uses these long-lived branch names:

- `main`: release entry, documentation, and metadata.
- `<version>-dev`: development for a specific version, for example `1.4.1-dev`.
- `<major>.<minor>.x-LTS`: a supported release series, for example `1.4.x-LTS`.

Develop against the latest appropriate `<version>-dev` branch. Do not make
source changes directly on `main` or an `*-LTS` branch. When creating a new
version development branch, start from the latest development branch and name
it for the version being developed:

```sh
git switch 1.4.1-dev
git switch -c 1.4.2-dev
```

Replace these version numbers with the actual base and target versions. Submit
a pull request to the appropriate development branch; release and LTS
synchronization are handled separately.

## Commits

Write commit messages in English. Use one behavior and one or more specific
domains in the subject, separating multiple domains with commas. Follow the
subject with a blank line and, when useful, a brief explanation. Then describe
zero or more affected modules in their own sections. After the module sections,
add optional sections such as Docs, Tests, or other cross-module concerns when
they help explain the change. Use plain text, `*` for bullets, and no nested
lists or Markdown formatting in the body.

```text
<behavior>(<domain>[, <domain>...]): <short description>

<optional brief explanation>

<optional module sections, zero or more>:
* <module-specific changes>

<optional cross-module sections, zero or more>:
* <changes or verification>
```

For example:

```text
feat(ipc): add process launch arguments

This commit adds an argument API for POSIX process launches.

process_launcher:
* Copy arguments before starting the child process.
* Reject unsupported arguments on Windows.

Tests:
* Check argument delivery and platform-specific failure behavior.
```

The body may be omitted for a change that is clear from its subject. Choose
section names for the actual components changed; do not use a generic subject
such as `fix: fix a bug`.

## Pull requests

Write pull requests in English and target the appropriate development branch.
Explain the problem solved or the new behavior, identify any breaking changes
(or state that there are none), and report the tests run and whether CI covers
and passes the change. Verify the required CI before requesting review.

Use [PR #32](https://github.com/JeongHan-Bae/JH-Toolkit/pull/32) as a
format example. Its body uses the following sections; add Notes only when
additional context, such as a backport or release plan, is relevant:

```markdown
### What problem does this PR solve / What is new in this PR?

Describe the problem and the resulting change.

### Is the PR backed by CI tests?

State the CI status and summarize relevant tests.

### Any breaking changes?

State whether the change is breaking and explain any migration needed.

### Notes

Add relevant context when needed.
```

Test changes on macOS with LLVM Clang and Ubuntu with GCC 13 or newer. Windows
with MinGW GCC must provide the API and honor its documented contract; platform
behavior may differ when documented. Explain any checks you could not run.

## Code style

The following guidelines apply to the JH-Toolkit core project. tiny-test and
simple-cucumber are separate frameworks with their own coding and header
conventions; follow the conventions of the framework being changed.

- Follow the existing module layout and STL-style names. Prefer practical,
  purpose-driven APIs over unnecessary generality.
- Maintain C++20 and `-fno-rtti` compatibility. Use `jh::meta::TStr` for
  compile-time string identities.
- Use concepts or constraints to reject invalid operations where practical;
  document any requirements that callers must enforce.
- Document public API behavior and caller obligations in Doxygen. Follow the
  header naming, copyright, and comment conventions in [AGENTS.md](AGENTS.md).
- Keep compile-time checks in `tests/static/` and runtime tests in the existing
  TDD or BDD modules. Add tests for observable behavior affected by a change.
