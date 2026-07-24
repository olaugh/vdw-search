#!/bin/sh
set -eu

if command -v sha256sum >/dev/null 2>&1; then
  sha256sum -c certs/SHA256SUMS
elif command -v shasum >/dev/null 2>&1; then
  shasum -a 256 -c certs/SHA256SUMS
else
  echo "Need sha256sum or shasum to verify certificate hashes." >&2
  exit 2
fi
