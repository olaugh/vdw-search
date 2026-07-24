#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
tectonic_bin=${TECTONIC:-tectonic}
output_dir="$repo_root/output/arxiv"
archive="$output_dir/vdw-search-arxiv-source.tar.gz"
test_pdf="$output_dir/vdw-search-arxiv-test.pdf"
work_dir=$(mktemp -d "${TMPDIR:-/tmp}/vdw-arxiv.XXXXXX")

cleanup() {
    rm -rf "$work_dir"
}
trap cleanup EXIT HUP INT TERM

source_dir="$work_dir/source"
initial_build_dir="$work_dir/initial-build"
test_source_dir="$work_dir/test-source"
test_build_dir="$work_dir/test-build"

mkdir -p "$source_dir" "$initial_build_dir" "$test_source_dir" \
    "$test_build_dir" "$output_dir"

cp "$repo_root/paper/main.tex" "$source_dir/main.tex"
cp "$repo_root/paper/references.bib" "$source_dir/references.bib"

"$tectonic_bin" "$source_dir/main.tex" \
    --outdir "$initial_build_dir" \
    --keep-logs --keep-intermediates
cp "$initial_build_dir/main.bbl" "$source_dir/main.bbl"

COPYFILE_DISABLE=1
export COPYFILE_DISABLE
(
    cd "$source_dir"
    tar -czf "$archive" main.tex references.bib main.bbl
)

tar -xzf "$archive" -C "$test_source_dir"
"$tectonic_bin" "$test_source_dir/main.tex" \
    --outdir "$test_build_dir" \
    --keep-logs --keep-intermediates
cp "$test_build_dir/main.pdf" "$test_pdf"

printf '%s\n' "arXiv source archive: $archive"
printf '%s\n' "clean-build test PDF: $test_pdf"
