#!/usr/bin/env sh
#
# Get the git repository root directory
REPO_ROOT=$(git rev-parse --show-toplevel)



# Change to the repository root directory
cd "$REPO_ROOT/src" || {
  echo "Failed to change to repository src directory: $REPO_ROOT/src" >&2
  exit 1
}

flawfinder --error-level=10 *.c
