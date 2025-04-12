#!/usr/bin/env python3
# scripts/update_compile_commands.py

import os
import subprocess
import sys


def main():
    print("Starting compile_commands.json generation...")

    # Get the git repository root directory
    try:
        repo_root = subprocess.check_output(
            ["git", "rev-parse", "--show-toplevel"], text=True
        ).strip()
    except subprocess.CalledProcessError:
        print("Error: Failed to get repository root", file=sys.stderr)
        return 1

    print(f"Repository root: {repo_root}")

    # Change to the repository root directory
    os.chdir(repo_root)
    os.chdir("src")

    print(f"Changed to directory: {os.getcwd()}")

    # Check if Makefile or any source files in src/ have changed
    try:
        changed_files = subprocess.check_output(
            ["git", "diff", "--cached", "--name-only"], text=True
        ).strip()
    except subprocess.CalledProcessError:
        print("Error: Failed to get changed files", file=sys.stderr)
        return 1

    relevant_changes = any(
        f
        for f in changed_files.split("\n")
        if f.startswith("src/")
        and f.endswith((".c", ".cpp", ".h", ".hpp"))
        or f == "Makefile"
        or f == "CMakeLists.txt"
    )

    if relevant_changes:
        print("Source files or build files changed, regenerating compile_commands.json")

        # Run bear
        print("Running: bear -- make clean all no-sanitize debug thread full")
        try:
            subprocess.run(
                [
                    "bear",
                    "--",
                    "make",
                    "clean",
                    "all",
                    "no-sanitize",
                    "debug",
                    "thread",
                    "full",
                ],
                check=True,
                stdout=sys.stdout,
                stderr=sys.stderr,
            )
            print("Successfully generated compile_commands.json")

            # Add to git
            subprocess.run(["git", "add", "compile_commands.json"], check=True)
            return 0
        except subprocess.CalledProcessError as e:
            print(
                f"Error: Failed to generate compile_commands.json: {e}", file=sys.stderr
            )
            return 1
    else:
        print("No relevant files changed, skipping compile_commands.json update")
        return 0


if __name__ == "__main__":
    sys.exit(main())
