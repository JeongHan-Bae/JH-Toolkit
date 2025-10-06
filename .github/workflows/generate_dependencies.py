import json
import re
import argparse
from pathlib import Path

# Locate project root
project_root = Path(__file__).resolve().parents[2]
dependencies_file = project_root / "dependencies.json"
cmake_file = project_root / "CMakeLists.txt"
test_cmake_file = project_root / "tests/CMakeLists.txt"


def extract_project_version(cmake_path):
    version_pattern = re.compile(r'set\s*\(\s*PROJECT_VERSION\s+([\d.]+)\s*\)', re.IGNORECASE)
    try:
        with open(cmake_path, "r", encoding="utf-8") as f:
            content = f.read()
            match = version_pattern.search(content)
            if match:
                return match.group(1)
    except FileNotFoundError:
        print(f"Warning: {cmake_path} not found.")
    return "unknown"


def extract_fetch_content_dependencies(test_cmake_path):
    fetch_pattern = re.compile(r'FetchContent_MakeAvailable\((\w+)\)')
    declare_pattern = re.compile(
        r'FetchContent_Declare\(\s*(\w+)\s+GIT_REPOSITORY\s+(.+?)\s+GIT_TAG\s+([^\s)]+)', re.DOTALL
    )

    dependencies = []
    declared = {}

    try:
        with open(test_cmake_path, "r", encoding="utf-8") as f:
            content = f.read()
            for match in declare_pattern.finditer(content):
                name, repo, version = match.groups()
                declared[name] = {"repo": repo, "version": version}

            for match in fetch_pattern.finditer(content):
                name = match.group(1)
                dep = declared.get(name, {"repo": "unknown", "version": "unknown"})
                dependencies.append({
                    "name": name,
                    "platforms": ["Ubuntu", "macOS", "Windows"],
                    "install": "Fetched automatically via CMake FetchContent in Debug and FastDebug builds",
                    "version": dep["version"],
                    "condition": "CMAKE_BUILD_TYPE=Debug or FastDebug",
                    "fetch_method": "FetchContent from GitHub",
                    "repository": dep["repo"]
                })
    except FileNotFoundError:
        print(f"Warning: {test_cmake_path} not found.")

    return dependencies


def main(get_version_only=False):
    version = extract_project_version(cmake_file)
    if get_version_only:
        print(version)
        return

    deps = extract_fetch_content_dependencies(test_cmake_file)
    data = {
        "project": {
            "name": "JH-Toolkit",
            "version": version,
            "description": "A cross-platform C++20 toolkit library",
            "platforms": ["Ubuntu", "macOS", "Windows"],
            "source": {
                "repository": "https://github.com/JeongHan-Bae/JH-Toolkit",
                "download": "git clone https://github.com/JeongHan-Bae/JH-Toolkit.git"
            }
        },
        "dependencies": [
                            {
                                "name": "C++20 Standard Library",
                                "platforms": ["Ubuntu", "macOS", "Windows"],
                                "install": "Requires modern C++20-compatible compilers: GCC 12+, Clang 15+. MSVC is not supported.",
                                "version": "C++20"
                            }
                        ] + deps
    }

    with open(dependencies_file, "w", encoding="utf-8") as f:
        json.dump(data, f, indent=2)

    print(f"Generated dependencies.json at {dependencies_file}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate dependencies.json or print version.")
    parser.add_argument("--get-version", action="store_true", help="Only print the project version and exit.")
    args = parser.parse_args()
    main(get_version_only=args.get_version)
