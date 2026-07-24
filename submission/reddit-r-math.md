# Draft r/math announcement

Replace `ARXIV_URL` after the preprint is public. Check the current r/math
rules immediately before posting.

## Suggested title

[Preprint] Six verifier-checked lower bounds for van der Waerden numbers

## Post body

I am the author of this preprint, which has not yet been peer reviewed:

**Preprint:** ARXIV_URL

**Verifier, certificates, and source:**

https://github.com/olaugh/vdw-search

The paper gives complete colorings establishing

- \(W(7,3)>344\);
- \(w(2;3,32)>1011\);
- \(w(2;3,33)>1069\);
- \(w(2;3,35)>1205\);
- \(w(2;3,36)>1259\); and
- \(w(2;3,39)>1420\).

To make the notation unambiguous, the paper uses **color-first notation**.
Thus \(W(r,k)\) means \(r\) colors and forbidden monochromatic progression
length \(k\). In particular, \(W(7,3)>344\) means that there is a
**7-coloring of \(\{1,\ldots,344\}\) containing no monochromatic 3-term
arithmetic progression**. It is unrelated to the much larger lower bound for
three colors avoiding 7-term progressions.

For the mixed notation \(w(2;3,t)\), the certificate is a 2-coloring in which
color 1 has no 3-term arithmetic progression and color 2 has no \(t\)-term
arithmetic progression.

The search combined focused stochastic local search with phase-seeded SAT
solving. The \(W(7,3)\) certificate came from a local-search state with one
remaining progression followed by a 25-position SAT-solver escape. Four of
the mixed certificates came from short extension ladders; the \(t=35\)
certificate required a coordinated two-position SAT repair.

Codex and Claude materially assisted with literature triage, implementation,
experiment orchestration, review, and manuscript preparation; Gemini caught
a notation ambiguity during review. The human author selected the problems,
set budgets, approved external actions, and takes responsibility for the
paper. No model output, solver timeout, or nearly satisfying state is treated
as mathematical evidence.

Every inequality is backed by a complete coloring accepted by a small
standalone exhaustive verifier that shares no code with the search programs.
From a fresh clone, the complete public verification is:

```text
make check
```

That command regenerates independent test fixtures, reproduces 12 published
source constructions byte for byte, verifies all 34 tracked complete
certificates, and checks their SHA-256 hashes.

The paper claims lower bounds only. It does not claim the next lengths are
unsatisfiable or that any of these van der Waerden numbers is known exactly.
I would especially welcome scrutiny of the verifier, the priority search, and
the notation.
