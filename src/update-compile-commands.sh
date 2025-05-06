#!/usr/bin/env sh
# Get the git repository root directory
REPO_ROOT=$(git rev-parse --show-toplevel)

# Log messages to ensure output is happening
echo "Starting compile_commands.json generation script" >&2

# Change to the repository root directory
cd "$REPO_ROOT/src" || {
  echo "Failed to change to repository src directory: $REPO_ROOT/src" >&2
  exit 1
}

echo "Changed to directory: $(pwd)" >&2

# Determine logical CPU count
logicalCpuCount=$([ "$(uname)" = 'Darwin' ] && sysctl -n hw.logicalcpu_max || lscpu -p | grep -E -v '^#' | wc -l)

# Check if source or build files changed

make clean

bear -- make all no-sanitize debug thread full -j "$logicalCpuCount"
bear make all no-sanitize debug thread full -j "$logicalCpuCount"

make clean
git add compile_commands.json
