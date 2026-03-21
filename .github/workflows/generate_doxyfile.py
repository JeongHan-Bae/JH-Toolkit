#!/usr/bin/env python3

import os
import subprocess
import pathlib
import sys
import argparse

DEFAULT_REPO_URL = "https://github.com/JeongHan-Bae/JH-Toolkit"


def normalize_repo(repo_env: str) -> str:
    if not repo_env:
        repo_env = DEFAULT_REPO_URL

    if repo_env.startswith("http"):
        repo_env = repo_env.rstrip("/")
        repo_env = repo_env.split("github.com/")[-1]

    return repo_env


def get_project_version() -> str:
    script_path = pathlib.Path(".github/workflows/generate_dependencies.py")

    if not script_path.exists():
        return "unknown"

    try:
        result = subprocess.run(
            [sys.executable, str(script_path), "--get-version"],
            capture_output=True,
            text=True,
            check=True,
        )
        version = result.stdout.strip()
        return f"v{version}" if version else "unknown"
    except subprocess.CalledProcessError:
        return "unknown"


def get_repo_description(repo: str, token: str) -> str:
    repo = normalize_repo(repo)

    curl_cmd = ["curl", "-s"]

    if token:
        curl_cmd.extend([
            "-H", f"Authorization: Bearer {token}",
            "-H", "Accept: application/vnd.github+json",
        ])

    curl_cmd.append(f"https://api.github.com/repos/{repo}")

    try:
        curl_proc = subprocess.run(
            curl_cmd,
            capture_output=True,
            text=True,
            check=True,
        )

        jq_proc = subprocess.run(
            ["jq", "-r", ".description // empty"],
            input=curl_proc.stdout,
            capture_output=True,
            text=True,
            check=True,
        )

        desc = jq_proc.stdout.strip()
        if not desc or desc == "null":
            return "C++ Toolkit for software engineering"

        return desc

    except Exception:
        return "C++ Toolkit for software engineering"


def escape_pipe(value: str) -> str:
    return value.replace("|", r"\|")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--no-html", action="store_true")
    parser.add_argument("--use-xml", action="store_true")
    args = parser.parse_args()

    generate_html = "NO" if args.no_html else "YES"
    generate_xml = "YES" if args.use_xml else "NO"

    project_version = get_project_version()

    repo_env = os.environ.get("GITHUB_REPOSITORY", "")
    token = os.environ.get("GITHUB_TOKEN", "")

    project_description = get_repo_description(repo_env, token)

    template_path = pathlib.Path("docs/doxy_settings/Doxyfile")
    output_path = pathlib.Path("Doxyfile.tmp")

    if not template_path.exists():
        print("Doxyfile template not found.", file=sys.stderr)
        sys.exit(1)

    content = template_path.read_text(encoding="utf-8")

    content = content.replace("__VERSION__", escape_pipe(project_version))
    content = content.replace("__DESCRIPTION__", escape_pipe(project_description))
    content = content.replace("__GENERATE_HTML__", generate_html)
    content = content.replace("__GENERATE_XML__", generate_xml)

    output_path.write_text(content, encoding="utf-8")

    print("Doxyfile generated.")
    print("Repo:", normalize_repo(repo_env))
    print("Version:", project_version)
    print("Description:", project_description)
    print("GENERATE_HTML:", generate_html)
    print("GENERATE_XML:", generate_xml)


if __name__ == "__main__":
    main()
