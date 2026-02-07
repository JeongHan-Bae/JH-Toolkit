#!/usr/bin/env python3
import pathlib
import os

ROOT = pathlib.Path("include/jh")

EXCLUDE_FILES = {}

EXCLUDE_DIRS = {}

def is_excluded(path: pathlib.Path) -> bool:
    if path in EXCLUDE_FILES:
        return True
    for d in EXCLUDE_DIRS:
        try:
            path.relative_to(d)
            return True
        except ValueError:
            pass
    return False


def is_ascii(data: bytes) -> bool:
    try:
        data.decode("ascii")
        return True
    except UnicodeDecodeError:
        return False


missing_copyright: list[str] = []
non_ascii_files: list[str] = []
bad_newline_files: list[str] = []

for p in ROOT.rglob("*"):
    if not p.is_file():
        continue
    if is_excluded(p):
        continue

    is_header = p.suffix == ".h"
    is_suffix_free = "." not in p.name

    if not (is_header or is_suffix_free):
        continue

    data = p.read_bytes()

    if not is_ascii(data):
        non_ascii_files.append(str(p))

    # newline-at-eof check: exactly one '\n'
    if not data.endswith(b"\n") or data.endswith(b"\n\n"):
        bad_newline_files.append(str(p))

    if is_header:
        text = data.decode("ascii", errors="ignore")
        if "@copyright" not in text:
            missing_copyright.append(str(p))


# ---------- Summary assembly ----------

sections: list[str] = []
sections.append("# Hygiene Check")

if missing_copyright:
    sections.append("## ⚠️ Missing `@copyright`")
    sections.append(
        "The following public header files do not contain a `@copyright` notice:"
    )
    sections.append("")
    for p in missing_copyright:
        sections.append(f"- `{p}`")
    sections.append("")

if non_ascii_files:
    sections.append("## ⚠️ Non-ASCII Public Headers")
    sections.append(
        "The following public headers contain non-ASCII characters. "
        "Public headers are required to be **pure ASCII** to ensure consistent "
        "cross-platform readability and tooling behavior:"
    )
    sections.append("")
    for p in non_ascii_files:
        sections.append(f"- `{p}`")
    sections.append("")

if bad_newline_files:
    sections.append("## ⚠️ Invalid EOF Newline")
    sections.append(
        "The following public headers must end with **exactly one `\\n`**. "
        "Files with no trailing newline or with multiple trailing newlines are invalid:"
    )
    sections.append("")
    for p in bad_newline_files:
        sections.append(f"- `{p}`")
    sections.append("")

if not missing_copyright and not non_ascii_files and not bad_newline_files:
    sections.append("## ✅ Header Hygiene OK")
    sections.append("All checked public headers passed hygiene checks.")


summary_path = os.environ.get("GITHUB_STEP_SUMMARY")
if summary_path:
    with open(summary_path, "a", encoding="utf-8") as f:
        f.write("\n".join(sections) + "\n")


# ---------- Warnings (signal only) ----------

if missing_copyright:
    print(
        f"::warning::{len(missing_copyright)} public header(s) missing @copyright"
    )

if non_ascii_files:
    print(
        f"::warning::{len(non_ascii_files)} public header(s) contain non-ASCII characters"
    )

if bad_newline_files:
    print(
        f"::warning::{len(bad_newline_files)} public header(s) have invalid EOF newline"
    )
