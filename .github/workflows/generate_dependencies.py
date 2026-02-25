import json
import re
import argparse
from pathlib import Path

# Locate project root
project_root = Path(__file__).resolve().parents[2]
dependencies_file = project_root / "dependencies.toml"
badge_file = project_root / "version_badge.json"
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
        r'FetchContent_Declare\(\s*(\w+)\s+GIT_REPOSITORY\s+(.+?)\s+GIT_TAG\s+([^\s)]+)',
        re.DOTALL
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
                    "condition": "Ubuntu/macOS: CMAKE_BUILD_TYPE=Debug or FastDebug.\n"
                                 "Windows (MinGW-UCRT): Fetched in any non-install build (including Release) to ensure stable testing without debug allocators.",
                    "fetch_method": "FetchContent from GitHub",
                    "repository": dep["repo"]
                })
    except FileNotFoundError:
        print(f"Warning: {test_cmake_path} not found.")

    return dependencies


def to_toml(data):
    lines = []

    # Project section
    project = data["project"]
    lines.append("[project]")
    for key in ["name", "version", "description", "platforms"]:
        value = project[key]
        if isinstance(value, list):
            value = "[" + ", ".join(f'"{v}"' for v in value) + "]"
            lines.append(f'{key} = {value}')
        else:
            lines.append(f'{key} = "{value}"')
    lines.append("")

    # project.source
    lines.append("[project.source]")
    for k, v in project["source"].items():
        lines.append(f'{k} = "{v}"')
    lines.append("")

    # dependencies list
    for dep in data["dependencies"]:
        lines.append("[[dependencies]]")
        for k, v in dep.items():
            if isinstance(v, list):
                v = "[" + ", ".join(f'"{x}"' for x in v) + "]"
                lines.append(f"{k} = {v}")
            else:
                if "\n" in v:
                    lines.append(f"{k} = '''")
                    lines.append(v)
                    lines.append("'''")
                else:
                    lines.append(f'{k} = "{v}"')
        lines.append("")

    return "\n".join(lines)


def main(get_version_only=False):
    version = extract_project_version(cmake_file)
    if get_version_only:
        print(version)
        return

    # Generate dependencies.toml
    deps = extract_fetch_content_dependencies(test_cmake_file)
    data = {
        "project": {
            "name": "JH-Toolkit",
            "version": version,
            "description": "An engineering-oriented C++20 toolkit with duck-typed concepts, "
                           "header-only, RTTI-free, and concurrency-friendly.",
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
                                "install": "Supports GCC 13+ and Clang 15+.\n"
                                           "Recommended compilers: \n"
                                           "GCC 14.2+ (official Ubuntu builds only; PPA builds are not recommended), 14.3+, or Clang LLVM 20 (stable release).",
                                "note": "LLVM 17/18 provided via Homebrew may not be fully stable and should be avoided in production, \n"
                                        "similar to the unrecommended GCC PPA builds.\n"
                                        "GCC 13 has known issues with NTTP combined with constexpr, certain APIs are intentionally disabled via static_assert checks. \n"
                                        "Attempting to call these APIs on GCC13 will result in a compilation failure.\n"
                                        "GCC 14+ has some known compile-time frontend parser issues (where correct syntax causes dangling), \n"
                                        "but these can be avoided by using our recommended syntax.\n"
                                        "The MinGW series (MinGW-w64/MinGW-clang) occasionally exhibits visibility issues when mixed with Windows runtimes (msvcrt/ucrt) \n"
                                        "(potentially stemming from SRWLock failing to produce total order even under seq_cst fences, \n"
                                        "with atomic types not fully protected). Windows serves as a verification platform rather than a high-concurrency platform.\n"
                                        "Some modules (refer to documentations) assumes POSIX semantics. The Windows implementation provides only the API, \n"
                                        "serving as a verification platform rather than a primary platform.\n\n"
                                        "Unsupported toolchains: \n"
                                        "MSVC, whose behavior partially deviates from ISO expectations and whose mangling conventions are rather peculiar.\n\n"
                                        "Highly recommended: \n"
                                        "LLVM 20. \n"
                                        "If no additional toolchain dependencies exist on Ubuntu, LLVM 20 can also be used as an alternative to GCC. \n"
                                        "On Windows, MinGW (GCC) + MSYS2 (UCRT) is recommended for minimal jitter and visibility gaps.\n"
                                        "(Generally, it will pull GCC 14.3+ or GCC 15).",
                                "version": "C++20"
                            }
                        ] + deps
    }

    toml_text = to_toml(data)

    with open(dependencies_file, "w", encoding="utf-8") as f:
        f.write(toml_text)

    print(f"✅ Generated {dependencies_file.name}")

    # Generate version_badge.json
    badge = {
        "schemaVersion": 1,
        "label": "JH-Toolkit",
        "message": version,
        "labelColor": "#555555",
        "namedLogo": "github",
        "color": "#559900",
        "style": "flat"
    }

    with open(badge_file, "w", encoding="utf-8") as f:
        json.dump(badge, f, indent=2)
    print(f"✅ Generated {badge_file.name}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate dependencies.toml and version_badge.json or print version.")
    parser.add_argument("--get-version", action="store_true", help="Only print the project version and exit.")
    args = parser.parse_args()
    main(get_version_only=args.get_version)
