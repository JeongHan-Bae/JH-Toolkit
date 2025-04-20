import json
import re
from pathlib import Path

# Locate the project root directory (assumes script is in .github/workflows/)
project_root = Path(__file__).resolve().parents[2]
dependencies_file = project_root / "dependencies.json"
cmake_file = project_root / "CMakeLists.txt"
test_cmake_file = project_root / "tests/CMakeLists.txt"

# Function to extract the project version from CMakeLists.txt
def extract_project_version(cmake_path):
    """
    Extracts the project version from set(PROJECT_VERSION x.y.z) in CMakeLists.txt.
    """
    version_pattern = re.compile(
        r'set\s*\(\s*PROJECT_VERSION\s+([\d.]+)\s*\)', re.IGNORECASE
    )

    try:
        with open(cmake_path, "r", encoding="utf-8") as _f:
            content = _f.read()
            match = version_pattern.search(content)
            if match:
                return match.group(1)  # Extracts version number like 1.3.0
    except FileNotFoundError:
        print(f"Warning: {cmake_path} not found.")

    return "unknown"


# Function to extract dependencies defined with FetchContent_MakeAvailable(...)
def extract_fetch_content_dependencies(test_cmake_path):
    """
    Extracts dependencies from test/CMakeLists.txt:
    - Finds FetchContent_MakeAvailable(...) dependencies.
    - Searches for matching FetchContent_Declare(...) entries to get repository URL and version.
    """
    fetch_pattern = re.compile(r'FetchContent_MakeAvailable\((\w+)\)')
    _dependencies = []
    declared_dependencies = {}

    try:
        with open(test_cmake_path, "r") as _f:
            content = _f.read()

            # Extract details from FetchContent_Declare(...)
            declare_pattern = re.compile(
                r'FetchContent_Declare\(\s*(\w+)\s+GIT_REPOSITORY\s+(.+?)\s+GIT_TAG\s+([^\s)]+)', re.DOTALL
            )
            for match in declare_pattern.finditer(content):
                name, repo, version = match.groups()
                declared_dependencies[name] = {"repo": repo, "version": version}

            # Extract FetchContent_MakeAvailable(...) dependencies
            for match in fetch_pattern.finditer(content):
                dep_name = match.group(1)
                dep_info = declared_dependencies.get(dep_name, {"repo": "unknown", "version": "unknown"})

                _dependencies.append({
                    "name": dep_name,
                    "platforms": ["Ubuntu", "macOS", "Windows"],
                    "install": "Debug mode only, automatically fetched via CMake FetchContent",
                    "version": dep_info["version"],
                    "condition": "CMAKE_BUILD_TYPE=Debug",
                    "fetch_method": "FetchContent from GitHub",
                    "repository": dep_info["repo"]
                })

    except FileNotFoundError:
        print(f"Warning: {test_cmake_path} not found.")

    return _dependencies

# Extract project version
project_version = extract_project_version(cmake_file)

# Extract dependencies from test/CMakeLists.txt
dependencies = extract_fetch_content_dependencies(test_cmake_file)

# Construct the dependencies.json structure
dependencies_data = {
    "project": {
        "name": "JH-Toolkit",
        "version": project_version,
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
                            "install": "Already included in modern compilers (g++-10+, clang++-10+, MSVC 19.28+)",
                            "version": "C++20"
                        }
                    ] + dependencies
}

# Write to dependencies.json
with open(dependencies_file, "w", encoding="utf-8") as f:
    f.write(json.dumps(dependencies_data, indent=2))


print(f"Generated dependencies.json at {dependencies_file}")
