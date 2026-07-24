#!/bin/sh
set -eu

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$repo_dir"

tmp_dir=$(mktemp -d "${TMPDIR:-/tmp}/vdw-reproduce.XXXXXX")
trap 'rm -rf "$tmp_dir"' EXIT HUP INT TERM
count=0

check_exact() {
  generated=$1
  tracked=$2

  ./verifier "$generated" >/dev/null
  if ! cmp -s "$generated" "$tracked"; then
    echo "FAIL: regenerated file differs from $tracked" >&2
    exit 1
  fi
  count=$((count + 1))
}

for pair in \
  "31 930" \
  "32 1006" \
  "33 1063" \
  "34 1143" \
  "35 1204" \
  "36 1257" \
  "37 1338" \
  "38 1378" \
  "39 1418"
do
  set -- $pair
  t=$1
  n=$2
  generated="$tmp_dir/aks_t${t}.txt"
  ./aks_expand "$t" "$n" "sources/aks_v4_t${t}.compact" "$generated"
  check_exact "$generated" "certs/aks_w2_3_${t}_gt${n}.txt"
done

generated="$tmp_dir/W2_7_gt3703.txt"
./gen_residue 617 2 7 0 1 >"$generated"
check_exact "$generated" certs/W2_7_gt3703_p617.txt

generated="$tmp_dir/W2_6_gt1131.txt"
./gen_residue 113 2 6 1 1 >"$generated"
check_exact "$generated" certs/W2_6_gt1131_p113_z1.txt

generated="$tmp_dir/W2_5_gt177.txt"
./gen_residue 11 2 5 2 1 >"$generated"
check_exact "$generated" certs/W2_5_gt177_p11_z2.txt

echo "published-certificate reproduction: $count byte-identical"
