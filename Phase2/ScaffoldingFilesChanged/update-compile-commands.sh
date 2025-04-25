#!/usr/bin/env sh
# Get the git repository root directory
REPO_ROOT=$(git rev-parse --show-toplevel)

# Log messages to ensure output is happening
echo "Starting compile_commands.json generation script for ScaffoldingFilesChanged" >&2

# Change to the repository root directory
cd "$REPO_ROOT/Phase2/ScaffoldingFilesChanged/" || {
  echo "Failed to change to repository src directory: $REPO_ROOT/Phase2/ScaffoldingFilesChanged/" >&2
  exit 1
}

echo "Changed to directory: $(pwd)" >&2


logicalCpuCount=$([ $(uname) = 'Darwin' ] && sysctl -n hw.logicalcpu_max || lscpu -p | grep -E -v '^#' | wc -l)

if git diff --cached --name-only | grep -q -E '(Makefile|src/.*\.(c|cpp|h|hpp)|CMakeLists.txt)'; then
  echo "Source files or build files changed, regenerating compile_commands.json"
  make clean
  bear -- make -j $logicalCpuCount
  git add compile_commands.json
  make clean
else
  echo "No relevant files changed, skipping compile_commands.json update"
fi
