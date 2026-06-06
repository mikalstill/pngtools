#!/bin/bash
# Populate fuzz/corpus/<target>/ from the PNG samples already in the
# repository. Both targets use the same seed set -- having more
# real-world starting points helps the coverage-guided mutator find
# new code paths quickly.

set -euo pipefail

cd "$(dirname "$0")"

mkdir -p corpus/fuzz_pngchunks corpus/fuzz_pnginfo

# Top-level sample images shipped with the source tree.
ROOT_SAMPLES=(
  ../sample.png
  ../input.png
  ../foursamplesperpixel.png
  ../multibytesample.png
  ../grayscale.png
)

# Generated test images, if tests/generate_test_images.py has been
# run. Glob expands to nothing if testdata/ is empty.
shopt -s nullglob
TESTDATA=( ../testdata/*.png )
shopt -u nullglob

count=0
for src in "${ROOT_SAMPLES[@]}" "${TESTDATA[@]}"; do
  [ -f "$src" ] || continue
  name=$(basename "$src")
  cp "$src" "corpus/fuzz_pngchunks/$name"
  cp "$src" "corpus/fuzz_pnginfo/$name"
  count=$((count + 1))
done

echo "Seeded $count file(s) per target."
