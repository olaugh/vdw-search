# Final notation, citation, and claim audit

Audit date: 2026-07-23

## Outcome

All six headline inequalities agree across the manuscript, README, records
table, complete certificate files, standalone verifier output, and SHA-256
manifest. No exact-value or unsatisfiability claim is made.

| Color-first claim | Physical object | Verifier | SHA-256 |
|---|---|---|---|
| `W(7,3) > 344` | 7-coloring of `[1,344]` avoiding monochromatic 3-APs | accepted | `d9f008e1a8985d51c70b8875b12df7520ad83ac4659670ef770493f95d1afb22` |
| `w(2;3,32) > 1011` | 2-coloring of `[1,1011]`; colors 1 and 2 avoid 3-APs and 32-APs | accepted | `abe37f598351ac49f07e31a819c3ca87cdfd86d1537b9988613a18dc3c7df8cc` |
| `w(2;3,33) > 1069` | 2-coloring of `[1,1069]`; colors 1 and 2 avoid 3-APs and 33-APs | accepted | `ddcb28d7aec8a88ca572f1da9cfcd3ffa78fff59463a7e7b09332c9069e29bcf` |
| `w(2;3,35) > 1205` | 2-coloring of `[1,1205]`; colors 1 and 2 avoid 3-APs and 35-APs | accepted | `de399156d8673b1c241cf22db41b3eb968e150e2f68478760b0eeaba068d9d61` |
| `w(2;3,36) > 1259` | 2-coloring of `[1,1259]`; colors 1 and 2 avoid 3-APs and 36-APs | accepted | `182465150650d344bac38accce1187789a5c0561100ff9cd55a3534ff472d2de` |
| `w(2;3,39) > 1420` | 2-coloring of `[1,1420]`; colors 1 and 2 avoid 3-APs and 39-APs | accepted | `11e5cdef316bdb2ad107921786c599553fceb60b33d68d37236700de63c87a88` |

## Notation

- The paper consistently uses color-first `W(r,k)`: `r` colors and forbidden
  progression length `k`.
- Mixed notation is `w(r;k_1,...,k_r)`, with forbidden length `k_i` assigned
  to color `i`.
- The text explicitly distinguishes the 7-color, 3-AP claim `W(7,3)>344`
  from the unrelated 3-color, 7-AP claim `W(3,7)>48,811`.
- Headline formulas are paired with plain descriptions of their physical
  colorings.

## Baselines and priority

- Ahmed--Kullmann--Snevily, arXiv:1102.5433v4 and the 2014 journal article,
  support `w(2;3,19)=349`, lower bounds through `t=39`, and the five mixed
  baseline certificate lengths used in the manuscript.
- Komkov, arXiv:1701.05603v5, is the cited source for `W(7,3)>343`.
- Monroe, arXiv:1603.03301v7, Rabung--Lotts, and the Leaps in Bounds tracker
  were checked to prevent a color-first/length-first collision and to confirm
  that `W(3,7)>48,811` is a different parameter pair.
- Exact-formula searches through 2026-07-23 found no later published
  certificate improving the six cited baselines. This supports the paper's
  qualified priority wording; certificate validity does not depend on the
  literature search being complete.

## Search narrative

Contemporaneous logs support the manuscript's reported solver versions,
timings, seed-relative phase method, 25-position `W(7,3)` escape, repeated
difference-171 frontier obstruction, one-flip `t=33` repair, and two-position
`t=35` SAT repair. Timeouts remain `UNKNOWN`.

## Reproducibility checks

The final batch ran:

```text
make check
```

It passed eight verifier regression tests, the mixed-search incremental-state
self-test, byte-identical regeneration of 12 published constructions,
verification of all 34 tracked complete certificates, and the full SHA-256
manifest.
