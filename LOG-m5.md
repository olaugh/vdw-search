# M5 Max T2 log — mixed w(2;3,t)

This log is owned by the M5/T2 lane. It never edits `LOG.md` or `LOG-m2.md`.
The existing `sls.c` is treated as M2/T4-owned. A rejected push is handled by
pull/rebase, checking that no concurrent work was overwritten, re-verifying
affected artifacts, and retrying; never force-push.

Nothing below is a mathematical result unless the frozen standalone
`verifier.c` accepts a saved certificate.

## 2026-07-23 — Phase M5-1 design: direct mixed SLS, starting at t=31

Target formula for an interval `[1,n]`, with one Boolean per element:

- color 1 must contain no 3-term AP, represented by an implicit negative
  3-clause for every `(a,a+d,a+2d)`;
- color 2 must contain no t-term AP, represented by an implicit positive
  t-clause for every `(a,a+d,...,a+(t-1)d)`.

Implementation will be a separate `t2_sls.c`, sharing no code with the
verifier or M2's T4 searcher. It will maintain per-element CSR incidence,
per-constraint true counts, an O(1) violated-constraint list, and optional
DDFW weights. A focused step chooses a violated constraint, evaluates flipping
each participating bit by weighted break/make cost, and applies a best move
with adaptive noise and tabu aspiration. DDFW redistributes weight at local
minima without changing the verifier-visible problem.

Independent restart classes will cover random, seed-derived, cyclic-mod-m,
palindromic, and digit-set templates. Palindromic and other templates are
streamliners only: any success must still be emitted as a complete ordinary
certificate and accepted by `verifier.c`.

Validation before search:

1. Compile with strict C11 warnings plus ASan/UBSan.
2. Cross-check maintained violation counts against a fresh brute-force recount
   after randomized flips on small instances.
3. Confirm any zero-violation state with the standalone verifier in a separate
   process before saving or reporting it.

Run protocol:

- `git pull --ff-only` before every batch;
- 18 independent single-threaded workers/restart classes;
- calibrate on solved smaller instances before t=31;
- no batch projected over one hour without John’s approval;
- apply the stopping rule only after cold-solve calibration and an actual
  best-violation history exist.

Phase M5-2 will independently transcribe and verify the AKS t=31..39
certificates, preserving certificate length and the paper's claimed bound as
separate fields. Phase M5-3 will add a separate CaDiCaL API lane with only safe
reflection lex-leaders in the base encoding; palindromic instances remain
explicitly separate lower-bound streamliners.

### Phase M5-1 implementation checkpoint

- Added independent `t2_sls.c`: implicit negative 3-clauses and positive
  t-clauses, CSR incidence, maintained true counts, O(1) violated list,
  focused WalkSAT and DDFW, tabu aspiration, adaptive noise, and random /
  seed / cyclic / palindrome / digit-sphere starting classes.
- The output deliberately labels itself `UNVERIFIED`; no verifier code is
  linked or copied into the search state.
- Strict C11 (`-Wall -Wextra -Wshadow -Wconversion -Wstrict-prototypes
  -pedantic -Werror`) build passed.
- ASan/UBSan self-test passed. The self-test performs 20,000 randomized flips
  and repeatedly compares maintained counts/list positions with a fresh
  enumeration of every 3-AP and t-AP.
- End-to-end regression only (NOT a new result): both WalkSAT and DDFW emitted
  w(2;3,4) candidates at n=17; the separately compiled `verifier.c` accepted
  them. Seed parsing and all five starting classes were exercised likewise.

## 2026-07-23 — Phase M5-2 design: AKS t=31..39 seed recovery

Use the final Ahmed–Kullmann–Snevily paper/source as the primary reference,
locate the authors' original certificate payload or generator data, and retain
enough provenance to reproduce every transcription. Do not infer a certificate
from a claimed table entry.

For each t=31..39:

1. Record the publication's claimed lower bound separately from the actual
   certificate interval length.
2. Convert the original representation to the frozen verifier format without
   sharing verifier code.
3. Run the standalone verifier on the complete converted certificate.
4. Save only verifier-accepted seeds under `certs/`; a failed or unavailable
   transcription is logged as such, never filled in heuristically.
5. Add an explicit `certificate length` and `claimed bound` field to
   `records.md`, keeping this shared-document update in its own clearly named
   commit.

### AKS t=31 recovered and independently verified

- Primary payload: arXiv:1102.5433v4 TeX source, appendix paragraph
  “w(2;3,31) > 930”; tracked verbatim as
  `sources/aks_v4_t31.compact`.
- Independent `aks_expand.c` expanded the paper's 0/1 run notation to exactly
  930 colors. AKS 0 (the block avoiding 3-APs) maps to verifier color 1;
  AKS 1 (the block avoiding 31-APs) maps to verifier color 2.
- Frozen standalone verifier output:
  `VALID: r=2 colors, lengths=(3,31), n=930`.
- Tracked certificate: `certs/aks_w2_3_31_gt930.txt`, SHA-256
  `d265a9631b348a89a34fb150b1bebc6bf1e8e5adbc424bacbeef47aee11412e4`.
- This reproduces the published lower bound and supplies the required t=31
  seed. It is not a new bound.

### AKS t=32..39 recovered and independently verified

The remaining eight appendix payloads were mechanically extracted from the
same arXiv v4 TeX source, expanded with `aks_expand.c`, checked against the
paper's stated interval length, and then accepted individually by the frozen
standalone verifier:

| t | claimed bound | cert length | certificate SHA-256 |
|---:|---:|---:|---|
| 32 | `> 1006` | 1006 | `4f2380e7cceda4f492bfce2df56c2ef2f19708e853e821812a09e006ad7b8105` |
| 33 | `> 1063` | 1063 | `b3c147e1cf0421b8892bbe741cd586e83265eba486007b4da5fbdf1113aeb732` |
| 34 | `> 1143` | 1143 | `801fa2221d8be26aa137a5b3281e26d885c8f3489c2d84dba44d64cfbb714fd0` |
| 35 | `> 1204` | 1204 | `ba0050506cf207c52e009a6b92e22f41b4f6c16765b6e8bfb4811f509bdf7f6e` |
| 36 | `> 1257` | 1257 | `8adf761cd1e9c7678d6bff08187e72393cb9fd9ade116f63bf0ccae32cac3aea` |
| 37 | `> 1338` | 1338 | `8215e8b7129ea2baab82ebbc261476cd5c12a78e43ef84fe5245e7036d0580c0` |
| 38 | `> 1378` | 1378 | `08e010eb1fac0b33eec2b3d4a8d5e7ac54690cd30e80f39cba4b22762b5176bc` |
| 39 | `> 1418` | 1418 | `140550d6664dc1d009bed17798b8536d24b87db1069bd181b1655720a075bae1` |

This completes the requested AKS seed recovery for t=31..39. These are
reproductions of published lower bounds, not improvements.

## 2026-07-23 — Phase M5-3 design: CaDiCaL API lane

Add a separate T2 CDCL generator, leaving the verifier and M2/T4 sources
untouched. Use Boolean `x_i=1` for the color avoiding 3-APs:

- every 3-AP contributes `(-x_a OR -x_b OR -x_c)`;
- every t-AP contributes `(x_a OR ... OR x_z)`.

Initialize CaDiCaL phases from a verifier-format AKS seed, extending beyond the
seed with a deterministic default. A SAT model is written only as an
`UNVERIFIED` complete certificate and must pass the standalone verifier before
tracking or reporting.

The ordinary/base instance may add the safe reflection lex leader
`x_1...x_n <=lex x_n...x_1`. Encode prefix equality with auxiliary variables
and prohibit `(x_i,x_{n+1-i})=(1,0)` only while every earlier reflected pair is
equal. Reflection is an automorphism of the interval/AP formula, so this keeps
at least one representative from each reflection orbit.

Palindromicity is a separate explicit mode adding
`x_i = x_{n+1-i}`. It is a search streamliner, not a safe ordinary-instance
symmetry break; any satisfying model still proves an ordinary lower bound only
because the unrestricted standalone verifier accepts the resulting complete
certificate.

Validation:

1. strict build against the installed CaDiCaL C API;
2. compare SAT/UNSAT against brute force for every n through the small
   w(2;3,3) and w(2;3,4) frontiers, with reflection on/off;
3. verify every SAT model separately with frozen `verifier.c`;
4. confirm the seed only changes phases, never clauses, by comparing clause
   counts and verdicts with/without `-i`.

### Phase M5-3 implementation checkpoint

- Installed Homebrew CaDiCaL 3.0.1 and added plain-C `t2_cadical.c` using
  `ccadical.h`; the final link uses CaDiCaL's static C++ library.
- Ordinary mode defaults to the reflection lex leader. `--no-reflection`
  removes it; `--palindrome` is a separate equality-streamlined instance.
- Validated 44 cases covering every nontrivial n from t through the exact
  w(2;3,3)=9 and w(2;3,4)=18 frontiers, with reflection both enabled and
  disabled: 40 SAT models were independently accepted by `verifier.c`, and
  all four boundary runs returned UNSAT.
- A palindromic w(2;3,3)>8 regression model was verifier-accepted.
- Seeded and unseeded n=17,t=4 encodings both reported the same 146 clauses;
  the seed affects CaDiCaL phases only.
- The termination callback returned UNKNOWN after 1.0004 seconds on the known
  hard n=349,t=19 boundary instance.
- Strict C11 compile and an ASan/UBSan seeded solve passed.

## 2026-07-23 — Phase M5-4 design: t=31 calibration and first attack

Start with t=31 only. Establish a cold-solve baseline before interpreting
failures at n=931:

1. Run an 18-process, short cold portfolio at n=919 (the largest point the AKS
   paper says its incremental local-search approach reached).
2. Confirm seed-derived behavior at the published n=930 certificate.
3. Attack n=931 with independent seed perturbations/reversal, random starts at
   several color-1 densities, cyclic moduli, palindromic starts, and
   digit-sphere starts; mix DDFW and focused WalkSAT. Run a seeded CaDiCaL
   instance as a separate lane without exceeding 18 simultaneous CPU-heavy
   processes.
4. Every SLS/CDCL process has an explicit wall-clock limit. Initial batches
   are minutes, not an hour; ask John before any projected wall time over one
   hour.
5. Keep raw logs under ignored `runs/`. If a generator emits a candidate,
   immediately run the frozen standalone verifier; commit/push only after
   acceptance.

The `~1000x` stopping rule cannot be instantiated until the cold portfolio has
an observed solve-cost distribution and a largest solved n. Record that
calibration first; do not manufacture a budget from the paper's historical
cutoffs.

### Cold calibration batches

- n=919, 18 independent 30-second starts: 0 solved. Best maintained violation
  count 59. Raw ignored logs: `runs/t31-cal-n919-20260723-m5/`.
- Ladder batch, six starts each for n=600/700/800, 30 seconds:
  - n=600: 5/6 solved in 4.97–17.25 seconds; every emitted model passed the
    frozen standalone verifier.
  - n=700: 0/6 solved; best violation count 9.
  - n=800: 0/6 solved; best violation count 24.
  Raw ignored logs: `runs/t31-cal-ladder-20260723-m5/`.

The n=600 models are calibration artifacts below the already tracked n=930
AKS certificate, not new lower bounds. The largest cold-solved n is not yet
pinned tightly enough for the 1000x stopping budget; narrow the 600–700 gap.

- Narrowing batch, six 60-second cold starts each:
  - n=625: 1/6 solved at 57.20 seconds; model accepted by frozen verifier.
  - n=650: 0/6 solved; best violation count 2.
  - n=675: 0/6 solved; best violation count 5.
  Raw ignored logs: `runs/t31-cal-n625-675-20260723-m5/`.

The current largest cold-solved n is 625 with observed solve cost 57.20 s.
The literal 1000x stopping allowance is therefore about 15.9 hours. That is
well over the one-hour approval threshold and is NOT authorized or scheduled.
Initial n=931 batches remain short probes far below that budget.

## 2026-07-23 — Phase M5-4a design: native and PGO search build

Before spending the first n=931 portfolio, build the T2 SLS lane with
`-O3 -march=native` and profile-guided optimization. Train the instrumented
binary on representative t=31 DDFW, focused-WalkSAT, and AKS-seeded workloads,
merge the profiles with `llvm-profdata`, and build with
`-fprofile-instr-use`.

PGO is a build-only search-speed optimization: it must not change the formula,
candidate format, or verifier. Validate the resulting binary with the built-in
incremental-state self-test and frozen standalone verifier regressions. Compare
fixed-work elapsed time against the existing release build before selecting the
binary for the 18-lane n=931 batch; retain PGO only if the measurement supports
it.
