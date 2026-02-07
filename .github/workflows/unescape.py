import os
import re
import html

INCLUDE_DIR = "include"

ENTITY_PATTERN = re.compile(r'(?<!\\)&[^;]+;')

def process_file(path: str) -> None:
    with open(path, "r", encoding="utf-8", errors="ignore") as f:
        content = f.read()

    def replace_entity(match: re.Match) -> str:
        return html.unescape(match.group(0))

    new_content = ENTITY_PATTERN.sub(replace_entity, content)

    if new_content != content:
        with open(path, "w", encoding="utf-8") as f:
            f.write(new_content)

def is_target_file(filename: str) -> bool:
    if "." not in filename:
        return True
    return filename.lower().endswith(".h")

def main() -> None:
    for root, _, files in os.walk(INCLUDE_DIR):
        for name in files:
            if is_target_file(name):
                path = os.path.join(root, name)
                process_file(path)

if __name__ == "__main__":
    main()
