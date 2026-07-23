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

## 2026-07-23 — Phase 1: frontier pinned

- Primary source: Monroe arXiv:1603.03301 v7 (Nov 2022), Tables 1-3 read in
  full (PDF). Rabung power-residue method EXHAUSTED over all primes <= 950M
  (500 CPU-years BOINC); cyclic zipping only swept to 40M (CPU-limited) —
  that 40M..950M zip gap is the main unexplored construction space.
- W(2,7) > 3703 CONFIRMED current; record is Rabung 1979 (p=617). Also
  W(2,10) > 103474 is Rabung 1979 (p=11497) — both survived the 950M sweep.
- No 2023-2026 concrete improvements found (searched; only asymptotic work:
  Hunter 2111.01099, Green FoM-Pi, 2301.06212 for r>=5).
- Mixed: AKS 1102.5433 (w(2;3,19)=349 exact; LBs t=20..39, conjectured exact
  only t<=30 -> t=31..39 soft). Ahmed JIS 2013: w(2;5,7)=260 exact.
- records.md written with full source table + target assessment:
  T1 W(2,7) via zip>40M / double-zip / SLS repair (marquee, low odds,
  honest reasons given), T2 w(2;3,31..39) (softest), T3 W(3,7)>48811
  (2016-era SAT bound, SLS-rangeable). Also W(2,8) rides the zip gap.
- STOPPED for Phase 1 review per protocol.

## 2026-07-23 — Phase 1 revision after external review (Fable 5)

Pasted Phase-1 summary to Fable; its critique verified claim-by-claim:
- Komkov 1701.05603 v5 (2020) MISSED by our search: 8 soft records
  (W(7,3)>343 ... W(5,6)>540197), certificates in the PDF. Added as T4.
- Zipper precondition CONFIRMED from Herwig-Heule EJC 2007 §5: base must
  be a valid cyclic certificate => zip sweep 40M..950M provably empty for
  power-residue bases. T1(a) killed. Zip-of-SLS-cyclic-base noted as the
  live Phase-4 variant; zipping only ever won for even r.
- Cost model fixed: Jacobi/Euler lazy evaluation, run-normalization
  (mono AP in Z_p <=> run of k consecutive equal characters) => valid
  primes exponentially rare; Monroe's k=7 null result explained.
- Ceiling argument retracted (self-contradictory ratio band).
- BCT 2018 recurrence added; Green/Hunter + NEW arXiv:2606.02541 (June
  2026, super-exponential 3-color growth) noted; AlphaEvolve/FunSearch
  collision check: none found; r=5,6 columns flagged for audit.
- REVISED portfolio: T2 (w(2;3,31..39)) heaviest, T4 Komkov family
  (new banker), T3 W(3,7) SLS, T1 reduced to multi-zips + repair at 3704+.
Phase 1 re-presented for approval.

## 2026-07-23 — Phase 2: reproduction complete

- gen_residue.c (plain C, zero shared code with verifier): Rabung base
  (free colors at residue-0 slots, base-r `choice` mask), Herwig 4-step
  zipper (z<=4), repetitive expansion. Free choices scanned by driver;
  verifier.c is the sole judge.
- REPRODUCED, all verifier-accepted, in certs/:
  * W(2,7) > 3703 (p=617, z=0, choice=1) — the 1979 marquee record.
  * W(2,6) > 1131 (p=113, z=1, choice=1) — matches known max (W=1132).
    choice=0 rejected with EXACTLY the predicted spacing-226 AP through
    the residue-0 copies — construction understanding confirmed.
  * W(2,5) > 177 (p=11, z=2) — double-zip path exercised, matches max.
  * Komkov W(7,3) > 343 and W(8,3) > 515 transcribed from the arXiv PDF,
    both verified first try — T4 SLS seeds ready.
- STOPPED for Phase 2 review.
