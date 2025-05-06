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

make clean
bear -- make -j "$logicalCpuCount"
bear make -j "$logicalCpuCount"


make clean
git add compile_commands.json
