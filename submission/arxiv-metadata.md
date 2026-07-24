# arXiv submission metadata

This file is deliberately ASCII-only so that its fields can be copied into
arXiv without introducing Unicode punctuation.

## Title

Verifier-Guided Human-AI Search for Six van der Waerden Lower Bounds

## Authors

John O'Laughlin

The PDF identifies the author as an independent researcher. Do not list any
AI system as an author.

## Abstract

We give explicit colorings establishing \(W(7,3)>344\) and
\(w(2;3,32)>1011\), \(w(2;3,33)>1069\), \(w(2;3,35)>1205\),
\(w(2;3,36)>1259\), and \(w(2;3,39)>1420\). Here and throughout, notation is
color first: \(W(r,k)\) concerns \(r\) colors and monochromatic \(k\)-term
arithmetic progressions, while \(w(r;k_1,\ldots,k_r)\) assigns forbidden
length \(k_i\) to color \(i\). Thus the first result is a 7-coloring of
\(\{1,\ldots,344\}\) with no monochromatic 3-term progression, and each
mixed result is a 2-coloring in which color 1 avoids 3-term progressions and
color 2 avoids \(t\)-term progressions. The search combined focused
stochastic local search with clause-weight redistribution and phase-seeded
conflict-driven clause learning. Every claim is represented by a complete
coloring and checked by a small standalone verifier that shares no code with
the search programs. We also describe the role of large-language-model
assistants in literature triage, software development, and experiment
orchestration. Generated prose and solver status were never treated as
evidence: the governing rule was certificate or nothing.

## Categories

- Primary: `math.CO` (Combinatorics)
- Cross-list: `cs.DM` (Discrete Mathematics)

The mathematical result is primary. Do not cross-list to `cs.AI`: the paper
uses AI assistants but does not introduce an artificial-intelligence method.

## MSC classification

`05D10`

## Comments

9 pages, 2 tables. Source code, standalone verifier, complete certificates,
and SHA-256 manifest: https://github.com/olaugh/vdw-search

## Fields to leave blank

- Journal reference
- DOI
- Report number

The arXiv and archival-software identifiers can be added after they exist.
