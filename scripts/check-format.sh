#!/bin/bash
set -e

cd "$(dirname "$0")/.."

# Check or fix C source formatting with clang-format.
# Usage:
#   scripts/check-format.sh        # check only (exits non-zero on diff)
#   scripts/check-format.sh fix    # reformat files in place

FILES=$(find . -maxdepth 1 -name '*.c' -o -name '*.h' | sort)

if [ "$1" = "fix" ]; then
    echo "==> Reformatting C sources..."
    clang-format -i $FILES
    echo "Done."
else
    echo "==> Checking C source formatting..."
    clang-format --dry-run --Werror $FILES
    echo "Formatting OK."
fi
