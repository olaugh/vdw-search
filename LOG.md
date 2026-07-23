# van der Waerden lower bounds — LOG

Working machine: 8-core Apple laptop (M5 Max 18-core handoff planned for sweeps).
Language plan (per John, final): plain C throughout unless a major library
need arises. Verifier is frozen C11 and never shares code with generators.

## 2026-07-23 — Phase 0: verifier

- verifier.c written first, standalone C11, no deps. Format: r / k_1..k_r / n /
  colors (1-based), '#' comments allowed. Scan enumerates every AP once via
  (first element, diff), testing the length assigned to the first element's
  color. Exit codes: 0 valid / 1 invalid (AP reported) / 2 parse error.
- tests/make_tests.py: INDEPENDENT python brute force cross-checks every case.
  8 tests: valid W(2,3)>8 w/ comments; exactly-one-hidden-AP (112211221, unique
  AP {1,5,9} d=4 — max-spread over n=9..11); all-same; valid mixed w(3,4;2)>17;
  same coloring with k-list swapped -> invalid (proves color->length mapping
  respected); color out of range -> exit 2; short color list -> exit 2;
  k=2 semantics (color may appear at most once).
- All 8 verdicts match brute force. Fact found: no 2-coloring of [1,12] has
  exactly one mono 3-AP (past W(2,3)=9 violations multiply).

Next: Phase 1 — pin the frontier (records.md), verify W(2,7)>3703 currency.
