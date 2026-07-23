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

## Target assessment

1. **T1 (marquee): W(2,7) > 3703** — current, and 47 years old. But honest
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
