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

### Native/PGO result

Apple Clang 21 trained the instrumented binary on t=31 random DDFW, random
focused-WalkSAT, and AKS-seeded DDFW workloads. The profile-use build passed
the built-in 20,000-flip state self-test; its DDFW and WalkSAT w(2;3,4)>17
candidates and the recovered AKS w(2;3,31)>930 certificate all passed the
frozen standalone verifier.

Three interleaved fixed-work rounds per mode (n=700, t=31, 100,000 flips)
gave these median elapsed times:

| build | DDFW | focused WalkSAT |
|---|---:|---:|
| existing release | 2.357 s | 1.690 s |
| `-march=native` | 2.320 s | 1.761 s |
| `-march=native` + PGO | 2.494 s | 1.865 s |

Matched seeds produced identical best/final violation counts and weight-update
counts in all builds. Native-only improves DDFW by about 1.6% in this small
sample. The trained PGO build is 5.8% slower than release for DDFW and 10.4%
slower for WalkSAT, so it is rejected rather than presumed beneficial. Use the
native-only binary for DDFW lanes and the existing release binary for WalkSAT
lanes in the first n=931 portfolio.

### First above-record t=31 batch

An 18-lane, 300-second n=931 portfolio completed with explicit per-process
limits and no residual workers:

- seeded reflected CaDiCaL: UNKNOWN at 300.001 s;
- seed-derived DDFW at perturbations 0,1,2R,3,5R,8: best violations
  1,1,21,56,62,63;
- seed-derived focused WalkSAT at perturbations 0R,1,2R,3,5R,8: best
  violations 1,15,73,32,59,76;
- random/structural DDFW (random density 15, cyclic q=31, digit q=10):
  best violations 69,69,71;
- random/structural focused WalkSAT (random density 20, palindrome density
  20): best violations 83,85.

The SLS lanes completed 2.12–2.24 million DDFW flips or 2.62–2.69 million
WalkSAT flips under the full 18-process load. No lane emitted a candidate, so
there was no certificate to accept, record, or claim. The three best=1
seed-neighborhood results show that the immediate n=930 seed boundary is far
more useful than the broad structural starts, but the current walks do not
retain or systematically explore that near-solution neighborhood. Raw ignored
logs: `runs/t31-n931-b1-20260723-m5/`.

## 2026-07-23 — Phase M5-5 design: bounded seed-neighborhood CDCL

The n=931 SLS batch reached but did not improve best=1 from three AKS-seeded
starts. An exhaustive diagnostic over all 931 insertion positions and both
inserted colors independently found no direct insertion extension: the minimum
is one violated progression. This is search guidance, not a certificate.

Add an optional CaDiCaL streamliner constraining the Hamming distance from the
seed-derived phase vector to at most K. Encode the cardinality bound with a
plain CNF sequential counter; do not share code with the verifier. Also permit
aligning a shorter seed to the right edge, so left- and right-aligned extension
neighborhoods can be searched symmetrically.

The Hamming bound is a search restriction, not a proof-safe symmetry break:
UNSAT means only that the selected neighborhood has no solution. A SAT model
is still a valid unrestricted lower-bound witness only after the frozen
standalone verifier accepts the complete emitted certificate. Validate the
counter against exhaustive small-instance Hamming-ball enumeration, test K=0
and K>=n boundaries, and rerun the existing unrestricted CaDiCaL regressions
before launching parallel radii.

### Bounded-neighborhood implementation checkpoint

`t2_cadical.c` now accepts `--hamming K` and `--seed-right`. The former adds a
Sinz sequential counter over mismatch literals; the latter right-aligns a
shorter certificate in the deterministic color-2 extension vector.

Validation completed before any n=931 use:

- 120 exhaustive small checks over eight independently generated seeds,
  n=5..8, t=3 or 4, both left/right alignment, and every K from 0 through n;
  CaDiCaL SAT/UNSAT exactly matched brute-force Hamming-ball enumeration;
- every SAT model from those checks stayed within K and passed the frozen
  standalone verifier;
- all 44 prior unrestricted reflection-on/off cases through the exact
  w(2;3,3)=9 and w(2;3,4)=18 frontiers retained their earlier verdicts, with
  all 40 SAT models verifier-accepted;
- an ASan/UBSan Hamming solve and verifier handoff passed.

### AKS n=931 neighborhood sweep, K=0..8

CaDiCaL exhaustively returned UNSAT for both the left-aligned extension
`seed[1..930],2` and its right-aligned reflection `2,seed[1..930]` at every
Hamming radius K=0 through 8. Reflection breaking was disabled, so each
verdict covers the complete selected ball (but is not global n=931 UNSAT).

| K | left solve | right solve |
|---:|---:|---:|
| 0 | 0.000005 s | 0.000003 s |
| 1 | 0.023 s | 0.025 s |
| 2 | 0.261 s | 0.243 s |
| 3 | 0.514 s | 0.491 s |
| 4 | 1.573 s | 1.137 s |
| 5 | 2.796 s | 3.192 s |
| 6 | 8.738 s | 7.997 s |
| 7 | 17.554 s | 15.784 s |
| 8 | 29.900 s | 31.945 s |

Thus no solution exists within eight recolorings of either endpoint-aligned
AKS extension. No candidate was emitted and no lower bound is claimed. Raw
ignored logs: `runs/t31-n931-hamming-k0-8-20260723-m5/`.

### AKS n=931 neighborhood sweep, K=9..17

- K=9: both complete balls UNSAT (left 137.949 s, right 80.221 s).
- K=10: left complete ball UNSAT in 137.046 s. The independent right solver
  timed out, but that ball is the exact reflection of the left ball because
  the n=930 AKS seed is palindromic; the mathematical neighborhood is
  therefore closed through K=10.
- K=11..17: both solver lanes reached the 300-second limit UNKNOWN.

No candidate was emitted and no lower bound is claimed. The exhaustive
neighborhood conclusion is only that any n=931 solution differs from either
endpoint-aligned AKS extension in at least 11 positions. Raw ignored logs:
`runs/t31-n931-hamming-k9-17-20260723-m5/`.

## 2026-07-23 — Phase M5-6 design: diversify the K=11 frontier

The first unresolved ball is K=11. Add narrow CaDiCaL search controls for its
documented random seed and `forcephase` option; neither changes the CNF.
Validate unchanged clause counts/verdicts and verifier-accepted SAT models on
small instances. Then use independent CaDiCaL seeds, split between ordinary
seed phasing and forced seed phasing, on K=11 only. This concentrates the next
bounded batch at the observed frontier rather than spending lanes on radii
already closed or substantially harder radii.

`t2_cadical.c` now exposes those controls as `--solver-seed N` and
`--force-phase`. A paired small Hamming instance retained exactly 22 variables,
51 clauses, and 134 literals with the controls on or off; both SAT models
passed the frozen verifier. The exact n=18,t=4 boundary remained UNSAT with
solver seed 17 and forced seed phasing. These controls affect CaDiCaL search
order only, not formula generation.

### Diversified K=11 batch

Eighteen independent 300-second n=931,K=11 CaDiCaL lanes used solver seeds
1100..1117, alternating the isomorphic left/right seed alignments and ordinary
versus forced AKS phasing. All 18 returned UNKNOWN. No candidate was emitted,
the frozen verifier had nothing to accept, and no lower bound is claimed.

This rules out the hypothesis that the first K=11 failures were merely one
unlucky CaDiCaL seed or early loss of the supplied phase vector; it does not
close the K=11 ball. Raw ignored logs:
`runs/t31-n931-hamming-k11-diverse-20260723-m5/`.

## 2026-07-23 — Phase M5-7 design: bank basins, pivot to t=36..39

The M2/T4 record run supplied directly relevant evidence: its best=1 SLS basin
required a verifier-accepted CDCL solution 25 flips away, beyond the small
Hamming-ball intuition. The orchestrator relay allocates at most one overnight
t=31 K=11/12 batch, which is not authorized under the one-hour approval rule;
do not schedule it now. The short t=31 lane is capped with the complete AKS
balls closed through K=10 and repeated K=11 UNKNOWN.

Move the majority T2 effort to t=36..39, beginning with each published bound
plus one. Extend `t2_sls` with an optional best-state output:

- copy the complete bit vector whenever the maintained violation count reaches
  a new minimum;
- on exhaustion, write that vector in certificate syntax but label it
  explicitly `NOT A CERTIFICATE`, including its exact maintained violation
  count and the still-violated progression(s);
- keep the zero-violation candidate path and exit semantics unchanged.

This is search-state banking, never a claimed result. Validate each banked
state by an independent violation counter (the frozen verifier should reject
nonzero states), then feed the complete coloring to unrestricted CaDiCaL as
phases. Run the t=36..39 SLS-to-basin and phase-seeded CDCL pipeline under
short explicit TTLs; only a full model accepted by `verifier.c` becomes a
certificate.

### Best-state banking implementation checkpoint

`t2_sls -b PATH` now snapshots every new minimum and, on exhaustion, writes
the best complete coloring with a prominent `NOT A CERTIFICATE` label. A
fresh progression enumeration independent of the maintained counters must
match the saved minimum before the file is written; all remaining 3-AP/t-AP
violations are listed in comments.

Validation:

- on the exact n=18,t=4 UNSAT boundary, a best=1 bank was independently
  counted as one violation, named its remaining 4-AP, and was rejected by the
  frozen verifier as required;
- CaDiCaL parsed that commented bank as phase input and retained the known
  UNSAT verdict;
- the ordinary zero-violation n=17,t=4 candidate path remained
  verifier-accepted and did not emit a misleading best-state file;
- built-in state self-test and ASan/UBSan bank/write/reject path passed.

### First high-t pipeline results: t=36 and t=39 improve immediately

The first t=36..39 portfolio produced two complete candidates before any
search flip: reversing each published AKS seed and appending the
seed-initializer's endpoint color directly extends:

- t=36 from n=1257 to n=1258;
  `certs/m5_w2_3_36_gt1258.txt`, SHA-256
  `4504e2110b293def09b823a3fb575fed702428e5fca097bbc86bd0b336dff944`;
- t=39 from n=1418 to n=1419;
  `certs/m5_w2_3_39_gt1419.txt`, SHA-256
  `93cbeb382bb343a0a2e526a11ab2643071c381155e91942babd3c765bcddb6f0`.

The frozen standalone verifier independently accepted both complete files:
`VALID: ... lengths=(3,36), n=1258` and
`VALID: ... lengths=(3,39), n=1419`. These strictly beat the AKS appendix
bounds by one. They are project results; a current-literature re-sweep remains
required before an external world-record claim. Other lanes in the bounded
portfolio were still running when these certificates were committed.

### Completed t=36..39 pipeline batch

All remaining stages finished under their 120-second per-stage limits:

| target | seed-SLS outcomes | direct/banked CDCL |
|---|---|---|
| t=36,n=1258 | reverse P0 SAT at 0 flips; other banks best 2 and 13 | all UNKNOWN |
| t=37,n=1339 | banks best 1, 1, and 3 | all UNKNOWN |
| t=38,n=1379 | banks best 5, 1, and 20 | all UNKNOWN |
| t=39,n=1419 | reverse P0 SAT at 0 flips; other banks best 1 and 12 | all UNKNOWN |

For t=37 and t=38, both endpoint colors were also tested with the original
and reversed AKS seed. None of those four direct extensions is valid; the
complementary-color probes can reach best=1 after one flip but not zero.

Thus this batch's only results are the already tracked and independently
verified t=36 and t=39 certificates. Every nonzero bank was rejected by the
frozen verifier before being used only as CaDiCaL phases. No residual workers
remained. Raw ignored logs: `runs/t2-high-t-pipeline-b1-20260723-m5/`.

### Direct climb: t=36 and t=39 improve by a second point

Exhausting original/reversed orientation and both endpoint colors at the next
length immediately found zero-flip color-2 extensions of the new certificates:

- `certs/m5_w2_3_36_gt1259.txt`: `VALID ... lengths=(3,36), n=1259`,
  SHA-256
  `182465150650d344bac38accce1187789a5c0561100ff9cd55a3534ff472d2de`;
- `certs/m5_w2_3_39_gt1420.txt`: `VALID ... lengths=(3,39), n=1420`,
  SHA-256
  `11e5cdef316bdb2ad107921786c599553fceb60b33d68d37236700de63c87a88`.

Both were accepted by the frozen standalone verifier before tracking. This
places each current project bound two points beyond its AKS appendix bound.
Raw ignored probes: `runs/t2-climb-endpoints-20260723-m5/`.

### Full-instance best=1 CDCL result

Eighteen 120-second CaDiCaL lanes used independently checked best=1 phase
states: all four endpoint-color/orientation basins for t=37 and t=38, plus
the next-length n=1260 and n=1421 basins for t=36 and t=39. Ordinary and
forced phasing were both represented. All 18 returned UNKNOWN; no candidate
was emitted and no result is claimed. No residual workers remained. Raw
ignored logs: `runs/t2-best1-cdcl-b1-20260723-m5/`.

## 2026-07-23 — Phase M5-8 design: violated-progression window CDCL

Add an optional CaDiCaL streamliner for a complete seed/banked state:

1. independently enumerate the seed's 3-AP and t-AP violations;
2. mark every element of each violated progression, plus an index radius R
   around each such element, as free;
3. add unit clauses fixing every other primary variable to the seed.

For a single violated long AP, R=0 frees t elements and R=1 frees at most 3t
elements, matching the relay's roughly 50–100-element local window. Report
the independently counted seed violations and actual free-variable count.

This is an unsafe search restriction: a SAT model proves the unrestricted
lower bound only after frozen-verifier acceptance, while UNSAT closes only
that bank/window. Validate generated windows against exhaustive small
enumeration and retain all unrestricted regressions before applying radii to
the t=37/t=38 best=1 banks.

### Window-CDCL implementation checkpoint

`t2_cadical --window R` independently enumerates seed violations, frees the
union of radius-R index neighborhoods around their progression elements, and
fixes every other primary variable with a unit clause.

Validation before real use:

- 48 exhaustive small cases over eight generated seeds, n=5..8, t=3/4,
  left/right alignment, and radii 0..2 exactly matched brute-force restricted
  SAT/UNSAT;
- every SAT model passed the frozen verifier and changed no fixed variable;
- all 44 unrestricted reflection-on/off frontier regressions retained their
  verdicts, with every SAT model verifier-accepted;
- ASan/UBSan passed a best=1 radius-1 window solve and correctly reported one
  independent seed violation and 12 free variables.

### Real best=1 window results

For all eight t=37/n=1339 and t=38/n=1379 endpoint-color/orientation
best=1 banks:

- radii 0,1,2,4,8 returned complete window UNSAT;
- radius 8 freed 619 of 1339 variables for t=37 and 638 of 1379 for t=38;
- radius 16 freed 1195/1339 and 1231/1379 respectively, but every lane
  reached 60 seconds UNKNOWN.

The best=1 next-length walls after the new records behaved similarly:
t=36,n=1260 windows at radii 1 and 4 were UNSAT, while radius 16
(1172/1260 free) was UNKNOWN; t=39,n=1421 radii 1 and 4 were UNSAT, while
radius 16 (1269/1421 free) was UNKNOWN. Direct original/reversed endpoint
probes with both colors and one search flip also produced no n=1260 or
n=1421 candidate.

Thus these basins have no repair even when roughly 46% of the t=37/t=38
interval is free around the violated progression; by radius 16 the window is
already close to the full hard instance. This is structural dead-basin
evidence, not a lower-bound result. No candidate was emitted and no residual
workers remained. Raw ignored logs:
`runs/t2-window-cdcl-b1-20260723-m5/`,
`runs/t2-window-cdcl-b2-20260723-m5/`, and
`runs/t2-window-cdcl-b3-20260723-m5/`.

### Endpoint sweep fills t=32 and t=33

The same finite original/reversed seed × endpoint-color sweep at each AKS
bound plus one produced two more zero-flip extensions:

- `certs/m5_w2_3_32_gt1007.txt`: frozen verifier
  `VALID ... lengths=(3,32), n=1007`, SHA-256
  `eae07d03332a8b6621e3274a75bcc73771a600acfba6e3c5aaa88b14605bb6de`;
- `certs/m5_w2_3_33_gt1064.txt`: frozen verifier
  `VALID ... lengths=(3,33), n=1064`, SHA-256
  `084b8285e1983f4c8949798c3bc3dc55e622fa80ad04034240a7374ab24e5576`.

No endpoint/one-flip candidate appeared for t=34,n=1144 or t=35,n=1205.
The t=32 and t=33 certificates improve their AKS appendix bounds by one;
literature re-sweep remains pending before an external record claim. Raw
ignored probes: `runs/t2-t32-35-endpoints-20260723-m5/`.

### Direct climb: t=32 and t=33 also improve by a second point

The current certificates again admit zero-flip color-2 endpoint extensions:

- `certs/m5_w2_3_32_gt1008.txt`: frozen verifier
  `VALID ... lengths=(3,32), n=1008`, SHA-256
  `1fde23651c3e4d03df0affe4f381162dba24a7b495d9fa688e666f92ec4c9fb6`;
- `certs/m5_w2_3_33_gt1065.txt`: frozen verifier
  `VALID ... lengths=(3,33), n=1065`, SHA-256
  `7f116680ef9fe8c108f4f044db827b0c3645889935d519d44285f3c51662812f`.

Both current project bounds are now two points beyond their AKS appendix
bounds. Raw ignored probes: `runs/t2-t32-33-climb2-20260723-m5/`.

### Direct climb: third points for t=32 and t=33

One more zero-flip color-2 endpoint extension gives:

- `certs/m5_w2_3_32_gt1009.txt`: frozen verifier
  `VALID ... lengths=(3,32), n=1009`, SHA-256
  `e7a8ffedd16471756b2b088f21d0949e11025e7b16dc028342b5c15b10fc1259`;
- `certs/m5_w2_3_33_gt1066.txt`: frozen verifier
  `VALID ... lengths=(3,33), n=1066`, SHA-256
  `3f09c6a2bb170c2a4c7a0eac48fdd3cdedd24a07de27200bb4e1677e2f17cc5e`.

The current project bounds are now three points beyond their AKS appendix
bounds. Raw ignored probes: `runs/t2-t32-33-climb3-20260723-m5/`.

### Direct climb: fourth points for t=32 and t=33

The fourth one-point batch extended both current certificates:

- `certs/m5_w2_3_32_gt1010.txt`: zero-flip color-2 endpoint extension;
  frozen verifier `VALID ... lengths=(3,32), n=1010`, SHA-256
  `9ad45712d46beef7678a5c9201aaf881bdc24d07644b37892471f22ea3ad884d`;
- `certs/m5_w2_3_33_gt1067.txt`: the color-2 endpoint seed began with one
  violation and WalkSAT repaired it in one flip; frozen verifier
  `VALID ... lengths=(3,33), n=1067`, SHA-256
  `90e66e6225c557b417e8fc6d72af06b901f0ec19c193d63b1eee1e1f9e667358`.

The current project bounds are now four points beyond their AKS appendix
bounds. Raw ignored probes: `runs/t2-t32-33-climb4-20260723-m5/`.
