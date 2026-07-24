#!/bin/sh
set -eu

verifier=${1:-./verifier}
count=0

expect_status() {
  expected=$1
  certificate=$2

  if "$verifier" "$certificate" >/dev/null 2>&1; then
    actual=0
  else
    actual=$?
  fi

  if [ "$actual" -ne "$expected" ]; then
    echo "FAIL: $certificate returned $actual, expected $expected" >&2
    exit 1
  fi
  count=$((count + 1))
}

expect_status 0 tests/t1_valid_w23_n8.txt
expect_status 1 tests/t2_one_hidden_ap.txt
expect_status 1 tests/t3_allsame_n5.txt
expect_status 0 tests/t4_valid_mixed_w34_n17.txt
expect_status 1 tests/t5_swapped_mixed_n17.txt
expect_status 2 tests/t6_color_out_of_range.txt
expect_status 2 tests/t7_wrong_count.txt
expect_status 1 tests/t8_k2_semantics.txt

uniform_output=$("$verifier" tests/t1_valid_w23_n8.txt)
mixed_output=$("$verifier" tests/t4_valid_mixed_w34_n17.txt)

case $uniform_output in
  *"colors=2; forbidden AP lengths=(3,3); color-first notation: W(2,3) > 8"*)
    ;;
  *)
    echo "FAIL: uniform success output does not lock color-first notation" >&2
    exit 1
    ;;
esac

case $mixed_output in
  *"colors=2; forbidden AP lengths=(3,4); color-first notation: w(2;3,4) > 17"*)
    ;;
  *)
    echo "FAIL: mixed success output does not lock color-first notation" >&2
    exit 1
    ;;
esac

echo "verifier regression tests: $count passed"
