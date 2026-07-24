# vdw-search

Verifier-checked lower-bound certificates and search tools for van der
Waerden numbers.

The repository's central rule is **certificate or nothing**: a search result
is not a mathematical result until the independent standalone verifier
accepts a complete coloring.

## Notation

This repository uses **color-first notation**:

- \(W(r,k)\) has \(r\) colors and forbids monochromatic \(k\)-term
  arithmetic progressions;
- \(w(r;k_1,\ldots,k_r)\) has \(r\) colors and color \(i\) forbids a
  \(k_i\)-term arithmetic progression.

Some literature instead writes the progression length first. In that
convention, the quantity written here as \(W(7,3)\) may be labeled
\(W(3,7)\). We therefore accompany every headline formula with the physical
coloring it asserts.

## Results

The project currently contains complete certificates for these improvements:

| Color-first quantity | Physical certificate | Previous bound | Certified here | Certificate |
|---|---|---:|---:|---|
| \(W(7,3)\) | 7 colors on \([1,344]\), avoiding monochromatic 3-APs | \(>343\) | **\(>344\)** | `certs/RECORD_W7_3_gt344.txt` |
| \(w(2;3,32)\) | 2 colors on \([1,1011]\), avoiding color-1 3-APs and color-2 32-APs | \(>1006\) | **\(>1011\)** | `certs/m5_w2_3_32_gt1011.txt` |
| \(w(2;3,33)\) | 2 colors on \([1,1069]\), avoiding color-1 3-APs and color-2 33-APs | \(>1063\) | **\(>1069\)** | `certs/m5_w2_3_33_gt1069.txt` |
| \(w(2;3,35)\) | 2 colors on \([1,1205]\), avoiding color-1 3-APs and color-2 35-APs | \(>1204\) | **\(>1205\)** | `certs/m5_w2_3_35_gt1205.txt` |
| \(w(2;3,36)\) | 2 colors on \([1,1259]\), avoiding color-1 3-APs and color-2 36-APs | \(>1257\) | **\(>1259\)** | `certs/m5_w2_3_36_gt1259.txt` |
| \(w(2;3,39)\) | 2 colors on \([1,1420]\), avoiding color-1 3-APs and color-2 39-APs | \(>1418\) | **\(>1420\)** | `certs/m5_w2_3_39_gt1420.txt` |

The mixed-number baseline is Ahmed, Kullmann, and Snevily (2014). The
\(W(7,3)\) baseline is Komkov (2020). The certificates above establish the
displayed inequalities independently of priority. A priority audit completed
on July 23, 2026 found no later published certificate improving any of the
six cited baselines; its scope and limitations are stated in the paper.

## Verify the results

Requirements for verification are deliberately small:

- a C11 compiler;
- `make`;
- POSIX `sh`;
- Python 3 for regenerating the small independent test fixtures.

From a fresh clone:

```sh
make verifier
./verifier certs/RECORD_W7_3_gt344.txt
./verifier certs/m5_w2_3_32_gt1011.txt
make check
```

`make check`:

1. compiles every dependency-free C program with warnings enabled;
2. regenerates the verifier test fixtures using an independent Python
   arithmetic-progression enumerator;
3. checks the verifier's valid, invalid, and malformed-input behavior;
4. runs the mixed-searcher's incremental-state self-test;
5. reproduces all nine AKS and three Rabung/zipper certificates byte-for-byte
   from their tracked compact sources or construction parameters;
6. verifies every complete certificate in `certs/`; and
7. checks every certificate against `certs/SHA256SUMS`.

The verifier does not share code with any generator or search program.

## Certificate format

Certificates are whitespace-separated integer files:

```text
r
k_1 ... k_r
n
c_1 ... c_n
```

Here `c_j` is the color of integer \(j\), in the range 1 through \(r\).
Lines whose first nonblank character is `#` are comments. The verifier exits
with status 0 for a valid certificate, 1 for an invalid coloring, and 2 for
malformed input.

## Repository layout

| Path | Purpose |
|---|---|
| `verifier.c` | Frozen, standalone exhaustive certificate verifier |
| `certs/` | Complete colorings accepted by the verifier |
| `certs/SHA256SUMS` | Integrity manifest for all tracked certificates |
| `seeds/` | Search states; explicitly **not** certificates |
| `sources/` | Compact source transcriptions used to reproduce published seeds |
| `tests/` | Independent verifier test generation and regression checks |
| `sls.c` | Uniform-\(k=3\) WalkSAT/DDFW searcher |
| `t2_sls.c` | Direct mixed \(w(2;3,t)\) WalkSAT/DDFW searcher |
| `t2_cnf.c` | Phase-seeded DIMACS generator for mixed instances |
| `t2_cadical.c` | Optional CaDiCaL API search lane |
| `cnf_linear.c`, `cnf_cyclic.c` | DIMACS emitters for linear and cyclic instances |
| `gen_residue.c` | Rabung/zipper construction generator |
| `aks_expand.c` | Expands the AKS appendix's compact certificate notation |
| `records.md` | Literature frontier and detailed result table |
| `LOG.md`, `LOG-m5.md` | Contemporaneous research logs, including failed paths and corrections |

The logs are provenance, not a source of certified claims. `records.md` and
the verifier-accepted files in `certs/` are the concise result record.

## Paper

The LaTeX manuscript is in [`paper/`](paper/). With
[Tectonic](https://tectonic-typesetting.github.io/) installed:

```sh
make paper
```

The final PDF is written to `output/pdf/vdw-search-paper.pdf`. The manuscript
states the physical coloring behind every headline formula and contains a
separate human–AI methods and disclosure section.

To produce and clean-build the minimal arXiv source archive:

```sh
make arxiv
```

This writes `output/arxiv/vdw-search-arxiv-source.tar.gz` and a PDF compiled
from a fresh extraction of that archive. Copy-paste submission metadata and
the announcement draft are tracked under [`submission/`](submission/).

## Building the search tools

Build the dependency-free tools:

```sh
make
```

For machine-specific search builds, supply the desired optimization flags:

```sh
make clean
make CFLAGS="-O3 -march=native -std=c11 -Wall -Wextra"
```

The verifier should remain a conventional, independently built binary; search
performance flags do not affect certificate validity.

The optional CaDiCaL lane requires CaDiCaL's headers and static library:

```sh
make t2_cadical CADICAL_PREFIX=/path/to/cadical/prefix
```

Kissat and CaDiCaL are search dependencies only. Neither is needed to verify
the certificates.

## Reproducibility and provenance

The `aks_` files reproduce certificate strings from Ahmed, Kullmann, and
Snevily's arXiv v4 appendix. The `komkov_` files are transcriptions of
certificates published by Komkov. See [NOTICE.md](NOTICE.md) for attribution
and licensing scope.

Search programs may emit candidates or best-so-far states. Only a complete
file accepted by `verifier` belongs in `certs/`. Files in `seeds/` remain
non-results even when they are one violated progression away from validity.

Generative-AI systems assisted with literature triage, experimental planning,
and code development. All mathematical lower-bound claims are backed by
complete certificates checked by the independent verifier.

## References

- M. Ahmed, O. Kullmann, and H. Snevily, “On the van der Waerden numbers
  \(w(2;3,t)\),” *Discrete Applied Mathematics* 174 (2014), 27–51.
  [arXiv:1102.5433](https://arxiv.org/abs/1102.5433)
- A. Komkov, “New Lower Bounds for van der Waerden Numbers,”
  [arXiv:1701.05603](https://arxiv.org/abs/1701.05603)
- D. Monroe, “New Lower Bounds for van der Waerden Numbers Using Distributed
  Computing,” [arXiv:1603.03301](https://arxiv.org/abs/1603.03301)

## Citation

Citation metadata for the manuscript and software is provided in
[CITATION.cff](CITATION.cff). Cite the paper together with an immutable tagged
release or archival DOI rather than the moving `main` branch. The arXiv and
Zenodo identifiers can be added to the metadata after submission and archival
release.

## License

Original software and documentation are released under the
[MIT License](LICENSE). Third-party source transcriptions are subject to the
attribution and scope statement in [NOTICE.md](NOTICE.md).
