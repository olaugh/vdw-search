#!/bin/sh
set -eu

verifier=${1:-./verifier}
count=0

for certificate in certs/*.txt; do
  if ! "$verifier" "$certificate" >/dev/null; then
    echo "FAIL: verifier rejected $certificate" >&2
    exit 1
  fi
  count=$((count + 1))
done

echo "certificate verification: $count accepted"
