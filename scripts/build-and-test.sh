#!/bin/bash
set -e

cd "$(dirname "$0")/.."

# Run autotools if configure doesn't exist
if [ ! -f configure ]; then
    echo "==> Running autotools..."
    aclocal
    autoconf
    automake --add-missing
    autoreconf
fi

# Run configure if Makefile doesn't exist
if [ ! -f Makefile ]; then
    echo "==> Running ./configure..."
    ./configure
fi

# Build
echo "==> Building..."
make -j"$(nproc)"

# Set up test venv if it doesn't exist
if [ ! -f tests/.venv/bin/stestr ]; then
    echo "==> Setting up test venv..."
    python3 -m venv tests/.venv
    tests/.venv/bin/pip install -r test-requirements.txt
fi

# Generate test images if needed
if [ ! -f testdata/with_text.png ]; then
    echo "==> Generating test images..."
    tests/.venv/bin/python tests/generate_test_images.py
fi

# Run tests
echo "==> Running tests..."
tests/.venv/bin/stestr run
