# Paper source

`main.tex` is the manuscript source and `references.bib` is its bibliography.
The paper deliberately uses color-first notation throughout:

- `W(r,k)` means `r` colors and forbidden progression length `k`;
- `w(r;k_1,...,k_r)` assigns forbidden length `k_i` to color `i`.

Build from the repository root with:

```sh
make paper
```

This uses Tectonic and writes the release-ready PDF to
`output/pdf/vdw-search-paper.pdf`. Temporary TeX files remain under
`tmp/pdfs/` and are ignored by Git.

Build and clean-test the minimal arXiv source archive with:

```sh
make arxiv
```

The archive contains `main.tex`, `references.bib`, and the generated
`main.bbl`. It is extracted into a fresh temporary directory and compiled
again before the target succeeds. The resulting archive and test PDF are
written under `output/arxiv/`.

Before an archival submission, freeze a tagged repository release containing
the paper, `verifier.c`, the six headline certificates, and
`certs/SHA256SUMS`.
