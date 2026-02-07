#!/usr/bin/env python3
import pathlib
import os

ROOT = pathlib.Path("include/jh")
DOCS_ROOT = pathlib.Path("docs")

EXCLUDE_FILES = {
    ROOT / "macros/header_begin.h",
    ROOT / "macros/header_end.h",
    }

EXCLUDE_DIRS = {
    ROOT / "detail",
    }

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


missing_headers: list[str] = []

for header in sorted(ROOT.rglob("*.h")):
    if is_excluded(header):
        continue

    # Map: include/jh/foo/bar.h -> docs/foo/bar.md
    rel = header.relative_to(ROOT)
    doc = (DOCS_ROOT / rel).with_suffix(".md")

    if not doc.is_file():
        missing_headers.append(str(header))


sections: list[str] = []
sections.append("# Presence Check")

if missing_headers:
    sections.append("## ⚠️ Missing Documentation Detected")
    sections.append("")
    sections.append(
        "The following public headers do not have corresponding documentation files:"
    )
    sections.append("")
    for h in missing_headers:
        sections.append(f"- `{h}`")
    sections.append("")
    sections.append(
        "> This check **does not** verify whether documentation content is up-to-date.\n"
        "> It only checks for **missing documentation files**."
    )
else:
    sections.append("## ✅ Documentation Presence OK")
    sections.append("")
    sections.append(
        "All public headers have corresponding documentation files."
    )


summary_path = os.environ.get("GITHUB_STEP_SUMMARY")
if summary_path:
    with open(summary_path, "a", encoding="utf-8") as f:
        f.write("\n".join(sections) + "\n")


# Emit warning but do NOT fail the job
if missing_headers:
    print(
        f"::warning::Missing documentation detected ({len(missing_headers)} files)"
    )
