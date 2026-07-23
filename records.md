# Van der Waerden frontier — records as verified 2026-07-23

Convention: W(r,k) = classical diagonal (r colors, AP length k); mixed
w(r; k1,...,kr). "LB x" means the published bound is "> x" with an explicit
certificate construction.

## Primary source for the classical table

Monroe, *New Lower Bounds for van der Waerden Numbers Using Distributed
Computing*, arXiv:1603.03301 **v7 (Nov 20, 2022)** — Tables 1–3. BOINC
project, ~500 CPU-years: **Rabung's power-residue method exhaustively swept
over ALL primes p ≤ 950,000,000** (for r ≤ 10, k ≤ 25); cyclic zipping swept
only to **p ≤ 40,000,000** ("terminated ... given CPU constraints").
No newer concrete improvements found in literature search (July 2026):
arXiv 2111.01099 (Hunter) and the Forum Math Pi paper (Green) are asymptotic
w(3,k); arXiv 2301.06212 is asymptotic r≥5; the 2025 SAT+CAS certificate
paper is Ramsey numbers, not vdW.

## Two colors, W(2,k)

| k | value / best LB | method (prime) | source | year |
|---|---|---|---|---|
| 3 | = 9 | — | Chvátal | 1970 |
| 4 | = 35 | — | Chvátal | 1970 |
| 5 | = 178 | quad-zipped p=11 achieves max | Stevens–Shantaram | 1978 |
| 6 | = 1132 | zipped p=113 achieves max | Kouril–Paul (exact) | 2008 |
| **7** | **LB 3703** | **Rabung, p=617** (6·617+1) | **Rabung** | **1979** |
| 8 | LB 11495 | zipped p=821 (2(7·821+1)−1) | Herwig–Heule–vL–vM | 2007 |
| 9 | LB 41265 | Herwig et al. | Herwig et al. | 2007 |
| 10 | LB 103474 | Rabung, p=11497 (9·11497+1) | Rabung | 1979 |
| 11 | LB 193941 | zipped p=9697 | Rabung–Lotts | 2012 |
| 12 | LB 638727 | zipped p=29033 | Rabung–Lotts | 2012 |
| 13–20 | LB 1642309 … 526317462 | quadratic residues | Liang–Xu–Shao–Baoxin | 2012 |
| 21–25 | LB 1409670741 … 23003662489 | Rabung (Monroe sweep) | Monroe | 2022 |

**Key structural facts (Monroe §1, §5, Table 2):**
- W(2,7) > 3703 dates to **1979** (p=617). The 950M-prime sweep found *no
  better prime for k=7*, and zips to 40M found nothing either. Pure Rabung
  is **method-exhausted below p = 950M** for every k ≤ 25.
- For k=8, the record is a **zip** (p=821): zips beat every pure-Rabung
  prime up to 950M there — zipping is genuinely stronger for some k, and its
  sweep frontier (40M) is 24× smaller than Rabung's (950M).
- Bound formulas: Rabung valid p ⇒ W(2,k) > (k−1)p+1; one zip ⇒ > 2((k−1)p+1)−1.
- Rabung validity needs only spacing-1 APs mod p + two endpoint conditions
  (Monroe Alg. 1) — the fast inner test for our sweeps.

## Three and four colors

| k | 3 colors | source | 4 colors | source |
|---|---|---|---|---|
| 3 | = 27 | Chvátal | = 76 | Beeler–O'Neil 1979 |
| 4 | = 293 | Kouril 2012 (exact) | LB 1048 | Rabung 1979 |
| 5 | LB 2173 | Heule–Mitchell–Nguyen 2009 (unpub.) | LB 17705 | Herwig et al. |
| 6 | LB 11191 | Heule–Mitchell–Nguyen 2009 | LB 157348 | Xu recursion |
| 7 | LB 48811 | **SAT@Delft web page, 2016** | LB 2284751 | Xu (3703×617) |
| 8 | LB 238400 | p=34057 (Monroe Table 2) | LB 12288155 | Xu |
| 9–12 | LB 932745 … 79134144 | Rabung–Lotts 2012 | … | Xu |
| 13–17 | LB 251282317 … 15509557937 | **Monroe 2022 (new)** | — | — |

## Mixed numbers (secondary targets)

- w(2;3,t): exact through **w(2;3,19) = 349** (Ahmed–Kullmann–Snevily,
  arXiv:1102.5433, final 2014, SAT/tawSolver). LBs for 20 ≤ t ≤ 39
  (e.g. w(2;3,20) ≥ 389); **conjectured exact only for t ≤ 30** — the
  t = 31..39 lower bounds are *not* believed tight by the authors.
- w(2;5,t): 178 (t=5, exact), 206 (t=6, Kouril 2006), **260 (t=7, exact,
  Ahmed JIS 2013** — 200 Opterons × 1 year for the UNSAT side).
  w(2;5,8) and beyond: no exact value found; frontier thin.
- w(2;4,t), w(2;6,t): Ahmed 2009/2011 (Integers) hold the tables; not yet
  re-derived here — pin down before targeting (TODO if selected).
- w(k; 2,...,2,s,t) family: Ahmed JIS 2013 Table 4 (exact, small n).

### AKS w(2;3,t) seed certificates recovered on M5

The paper's claimed bound and the actual expanded certificate length are
recorded separately. “Accepted” means the frozen standalone `verifier.c`
accepted the tracked certificate, not merely that its length matched the
paper.

| t | paper's claimed bound | certificate length | tracked certificate | verifier |
|---:|---:|---:|---|---|
| 31 | `w(2;3,31) > 930` | 930 | `certs/aks_w2_3_31_gt930.txt` | accepted |
| 32 | `w(2;3,32) > 1006` | 1006 | `certs/aks_w2_3_32_gt1006.txt` | accepted |
| 33 | `w(2;3,33) > 1063` | 1063 | `certs/aks_w2_3_33_gt1063.txt` | accepted |
| 34 | `w(2;3,34) > 1143` | 1143 | `certs/aks_w2_3_34_gt1143.txt` | accepted |
| 35 | `w(2;3,35) > 1204` | 1204 | `certs/aks_w2_3_35_gt1204.txt` | accepted |
| 36 | `w(2;3,36) > 1257` | 1257 | `certs/aks_w2_3_36_gt1257.txt` | accepted |
| 37 | `w(2;3,37) > 1338` | 1338 | `certs/aks_w2_3_37_gt1338.txt` | accepted |
| 38 | `w(2;3,38) > 1378` | 1378 | `certs/aks_w2_3_38_gt1378.txt` | accepted |
| 39 | `w(2;3,39) > 1418` | 1418 | `certs/aks_w2_3_39_gt1418.txt` | accepted |

## Corrections from external review (Fable) — verified against sources

1. **Komkov (arXiv:1701.05603 v5, Oct 2020)** — MISSED in first pass; now
   verified from the PDF (certificates embedded in the paper):
   W(7,3) > 343, W(8,3) > 515, W(10,3) > 892, W(11,3) > 1187,
   W(17,3) > 3549 (all beating Heule 2017 "Avoiding Triples" by +1..+4),
   plus W(7,4) > 9980, W(6,5) > 99554, W(5,6) > 540197 (beating his own
   2018 GA bounds; last two certs at komkov.org/VanDerWaerden/).
   Lone-author SAT+GA, hill-climb margins, small n: softest records found.
2. **Zipper precondition (Herwig-Heule EJC 2007, §5, verified)**: the
   Cyclic Zipper method's step 1 is "suppose a CYCLIC CERTIFICATE of
   length n is found" — the base must already be valid. Consequence: for
   W(2,7) the valid power-residue bases below 950M are fully known
   (largest p=617, Monroe), all were zipped long ago, so a zip sweep of
   primes 40M–950M is PROVABLY EMPTY for power-residue bases. T1(a) dead.
   Live variant: the base may come from "any other technique" — zipping
   SLS-found cyclic certificates is unexplored. Also (ibid.): zipping has
   only ever produced records for EVEN r; bound formula (k−1)·p·2^z + 1.
3. **Cost model corrected**: no O(p) discrete-log array needed — r=2
   coloring is the quadratic character (Jacobi symbol, O(log p)/element);
   general r via Euler's criterion t^((p−1)/r) mod p. Lazy prefix testing
   ≈ microseconds/prime. And by multiplicativity, a mono AP of difference
   d in Z_p normalizes to a RUN of k consecutive equal characters, so
   expected violations grow ~ p·r^(1−k): valid primes thin out
   exponentially — explaining why records come from tiny primes and the
   950M sweep found nothing at k=7.
4. **Ceiling folklore retracted**: my "3400–4500" band contradicted its
   own data (3703/1132 ≈ 3.27 > 2.7). No reliable ceiling estimate exists.
5. **Blankenship–Cummings–Taranchuk (EJC 69, 2018)**: W(k,r) >
   p·(W(k, r−⌈r/p⌉) − 1) for prime p ≤ k — generalizes Berlekamp; feeds
   several multicolor entries (used in Monroe Tables 1–3).
6. **Asymptotic context** (audience value for T2): Green (FoM Pi 2022)
   refuted AKS's w(3,k)=O(k²) conjecture (w(3,k) ≥ k^{b(k)}, b→∞);
   Hunter (2111.01099) improved b(k); arXiv:2606.02541 (June 2026)
   claims super-exponential growth for THREE-color w(3;k,k,k) — skim
   before publishing anything in that family.
7. **AlphaEvolve/FunSearch collision check**: no published vdW-type
   sweep found (cap sets, kissing numbers, etc. — not vdW). Re-verify
   against Tao's Nov 2025 "at scale" problem list before M5 Max commit.
8. **r=5,6 columns need an audit**: Herwig 2007 Table 3/4 entries
   (e.g. W(6,6) > 633981, zipped p=31699) may still be current and soft;
   Komkov superseded W(6,5). Cross-check against BCT/Xu recursions and
   OEIS before targeting.

## Target assessment (REVISED after review)

Priority order: **T2 heaviest, T4 (new) as second banker, T3, T1 reduced.**

- **T2 (banker #1): w(2;3,t), t=31..39** — unchanged, strengthened by
  Green/Hunter/2606.02541 (the tightness conjecture's foundation is gone).
- **T4 (banker #2, added): the Komkov family** — W(7,3) > 343,
  W(8,3) > 515, W(10,3) > 892, W(11,3) > 1187, W(17,3) > 3549, and his
  W(7,4)/W(6,5)/W(5,6). 2020 SAT/GA margins of +1..+4 on small n; also
  W(6,6) > 633981 (Herwig 2007) pending the r=5,6 audit.
- **T3: W(3,7) > 48811** — direct focused-walk SLS at n≈49k, seeded from
  the existing certificate and character constructions; palindromic /
  cyclic restart classes (sound for lower bounds; Herwig: certificates
  show strong symmetry regularities).
- **T1 (reduced, ~10%): W(2,7) > 3703** — (a) zip sweep DEAD (see
  corrections); survives: systematic multi-zips of small valid bases,
  zips of SLS-found cyclic bases (even-r caveat noted), and SLS repair at
  n=3704–3800 seeded from the p=617 coloring — which doubles as evidence
  about the true ceiling that no ratio heuristic provides.

Original T1 notes kept below for the record:

1. **T1 (old marquee): W(2,7) > 3703** — current, and 47 years old. But honest
   odds assessment: pure Rabung exhausted to 950M; growth-ratio evidence
   (Monroe Fig. 1: consecutive-k LB ratios oscillate 2–2.7) puts plausible
   true W(2,7) around 3–4 × W(2,6)=1132, i.e. 3400–4500 — 3703 may be near
   the ceiling. Attack surface that is genuinely unexplored: **zips of
   primes 40M–950M** (Monroe stopped at 40M for CPU reasons), **double zips
   at moderate primes** (only ever applied to tiny cases), SLS repair
   starting from the p=617 coloring extended past 3703.
2. **T2 (softest): w(2;3,t) for t = 31..39** — AKS's own authors do NOT
   conjecture these LBs exact. Certificates are tiny (n ≈ 600–1000); 2026
   SLS/SAT on one laptop dwarfs 2011 tawSolver runs. Best odds of a strict
   improvement; smaller headline.
3. **T3: W(3,7) > 48811** — a 2016-era SAT@Delft web-page bound, not
   obviously method-exhausted; n ≈ 49k is in SLS range; also 3-color zips
   beyond 40M are unexplored (same gap as T1).

Honorable mention: W(2,8) > 11495 (2007 zipper record; zip search beyond
40M applies here too and zips are known to be the winning method at k=8).
