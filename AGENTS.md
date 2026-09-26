# Agent Instructions

These instructions apply to the whole repository and to every coding agent working here. Read this file and `CONTRIBUTING.md` before making changes. Follow both; also follow any more specific `AGENTS.md` in a subdirectory. The contribution guide remains the source of detailed coding, commit, and PR conventions.

## Project layout

- `include/jh/`: public C++ headers and library API; the project is fully header-only.
- `src/`: precompiled translation units and helpers for template instantiation, not the general implementation home.
- `tests/static/`, `tests/tdd/`, `tests/bdd/`: compile-time, unit, and behavior tests.
- `simple-cucumber/`: Gherkin/BDD test support.
- `examples/`: usage examples; `docs/`: user and API documentation.
- `cmake/`, `tools/`: build helpers and tooling.

Keep changes in the appropriate area and preserve the existing module and test organization.

## Header copyright

Every header must have a file-level Doxygen block containing `@file` and `@brief` before `#pragma once`. Non-aggregate headers that provide valid library code require the copyright block and fixed `@author` annotation shown below. Put the copyright block first and the file-level block immediately after it, with no blank line between them. Aggregate headers, translation units (TUs), instantiation helpers, and tests do not need copyright blocks. Test headers may omit `@author`; if included, it may name the actual contributor or contributors, using the same escaped HTML email-link format as the fixed annotation, for example `@author Contributor <a href="mailto:name&#64;example.com">&lt;name\@example.com&gt;</a>`. Put one blank line between the file-level block and `#pragma once`. Preserve the entities and escapes exactly.

Use this exact order and content for a non-aggregate header that provides valid library code:

```text
/**
 * @copyright
 * Copyright 2025 JeongHan-Bae &lt;mastropseudo\@gmail.com&gt;
 * <br>
 * Licensed under the Apache License, Version 2.0 (the "License"); <br>
 * you may not use this file except in compliance with the License.<br>
 * You may obtain a copy of the License at<br>
 * <br>
 *     http://www.apache.org/licenses/LICENSE-2.0<br>
 * <br>
 * Unless required by applicable law or agreed to in writing, software<br>
 * distributed under the License is distributed on an "AS IS" BASIS,<br>
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.<br>
 * See the License for the specific language governing permissions and<br>
 * limitations under the License.<br>
 * <br>
 * Full license: <a href="https://github.com/JeongHan-Bae/JH-Toolkit?tab=Apache-2.0-1-ov-file#readme">GitHub</a>
 */
/**
 * @file example.hpp
 * @brief Brief description of the header.
 * @author JeongHan-Bae <a href="mailto:mastropseudo&#64;gmail.com">&lt;mastropseudo\@gmail.com&gt;</a>
 */

#pragma once
```

## Documentation comments

- Document the behavior of every public API with at least a brief comment explaining what it does. Internal implementation details may be left undocumented or given a short, single-line `///` comment.
- Keep test comments minimal. Add a short comment only when the test's intent or setup is not clear from the test itself; tests do not need public-API-style documentation.
- Write API comments in the project's Doxygen style. Use native Doxygen annotation commands such as `@brief`, `@param`, `@tparam`, and `@return`.
- Start every multi-line Doxygen block with `/**` on its own line; put `@file`, `@brief`, and other commands on following lines. Use `/// @brief ...` for single-line comments. Do not write `/** @xxx` or trailing `///<` comments.
- Use HTML tags for formatting and lists: `<ul><li>...</li></ul>` or `<ol><li>...</li></ol>`. Do not use Markdown bullet lists or numbered lists such as `1.`, `2.`, `3.` in comments.
- Do not use Markdown bold (`**text**`); use HTML bold (`<b>text</b>`).
- Use `<code>...</code>` for inline code and `@code...@endcode` for multiline code. Escape special characters inside code, especially `<` and `>` in C++ template examples (for example, `std::vector&lt;int&gt;`).
- In semantic prose, do not use `->` or `→` as arrows; write `&rarr;` instead. This rule does not apply inside code or code examples.
- Write check and cross marks as HTML entities, such as `&#x2713;` and `&#x2717;`. Write tables with HTML `<table>` markup; do not use Markdown pipe tables.
- Avoid unnecessary escaping in ordinary prose and write HTML tags directly.

## Branches and changes

- Develop on the latest `<version>-dev` branch. Do not make direct source changes on `main` or `*-LTS`.
- Keep changes focused and production-relevant. Follow `CONTRIBUTING.md` for project-specific design, naming, documentation, and verification requirements.

## Commits

- Never run `git commit` without the developer's explicit confirmation of the proposed message.
- Before requesting confirmation, write the complete commit message in English to the repository-root `commit.message.tmp`, following the exact format and body rules in `CONTRIBUTING.md`.
- Show the developer the complete contents of `commit.message.tmp` and a concise change summary, then wait. Do not commit before confirmation; after confirmation, commit only as directed by the developer.

## Pull requests

Prepare PRs in English and follow `CONTRIBUTING.md`. Include: the problem solved or change made, whether CI tests back the change, and any breaking changes. Target the appropriate development branch and verify the required CI before requesting review.
