#!/usr/bin/env bash

set -euo pipefail

mode="check"
if [[ "${1:-}" == "--fix" ]]; then
    mode="fix"
fi

mapfile -t files < <(
    git ls-files '*.c' '*.cc' '*.cpp' '*.h' '*.hpp' '*.inl' \
    | grep -vE '(^|/)(g3d|bullet_physics|rbxg3d)/' \
    | grep -vE '(^|/)Generated/' \
    | grep -vE '^libs/'
)

if [[ ${#files[@]} -eq 0 ]]; then
    echo "clang-format: no first-party files to check."
    exit 0
fi

echo "clang-format: ${#files[@]} first-party files (vendored + generated excluded)."

if [[ "${mode}" == "fix" ]]; then
    clang-format -i "${files[@]}"
    echo "clang-format: applied."
else
    clang-format --dry-run --Werror "${files[@]}"
    echo "clang-format: clean."
fi
