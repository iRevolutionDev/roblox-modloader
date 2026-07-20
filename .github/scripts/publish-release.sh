#!/usr/bin/env bash
set -euo pipefail

usage() {
    echo "usage: publish-release.sh <tag> <title> <artifact_dir> [--prerelease]" >&2
    exit 2
}

[ $# -ge 3 ] || usage

tag="$1"
title="$2"
artifact_dir="$3"
shift 3

prerelease=()
if [ "${1:-}" = "--prerelease" ]; then
    prerelease=(--prerelease)
fi

[ -d "$artifact_dir" ] || { echo "artifact dir not found: $artifact_dir" >&2; exit 1; }

shopt -s nullglob
zips=("$artifact_dir"/*.zip)
if [ ${#zips[@]} -eq 0 ]; then
    echo "no .zip artifacts in $artifact_dir" >&2
    exit 1
fi

(cd "$artifact_dir" && sha256sum ./*.zip | sed 's| \./| |' > SHA256SUMS.txt)
cat "$artifact_dir/SHA256SUMS.txt"

notes=$(cat <<EOF
Built from \`${SOURCE_SHA:-unknown}\`.

[Producing workflow run](${SOURCE_RUN_URL:-${GITHUB_SERVER_URL:-https://github.com}/${GITHUB_REPOSITORY:-}/actions})

SHA256:
\`\`\`
$(cat "$artifact_dir/SHA256SUMS.txt")
\`\`\`
EOF
)

if gh release view "$tag" >/dev/null 2>&1; then
    gh release view "$tag" --json assets --jq '.assets[].name' | while read -r asset; do
        [ -n "$asset" ] || continue
        gh release delete-asset "$tag" "$asset" --yes
        echo "removed stale asset: $asset"
    done
    gh release edit "$tag" --title "$title" --notes "$notes" ${prerelease[@]+"${prerelease[@]}"}
else
    gh release create "$tag" --title "$title" --notes "$notes" ${prerelease[@]+"${prerelease[@]}"}
fi

gh release upload "$tag" "${zips[@]}" "$artifact_dir/SHA256SUMS.txt" --clobber
