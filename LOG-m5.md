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
