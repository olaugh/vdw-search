/* cnf_cyclic.c — DIMACS emitter: does an r-coloring of Z_m (m odd) with no
 * monochromatic cyclic 3-AP exist? SAT model = cyclic core (decode: var
 * x_{i,c} = (i-1)*r + c, i in 1..m, c in 1..r). Symmetry: position 1 is
 * fixed to color 1. Generator-side tooling; any resulting core is unrolled
 * to 2m+1 and judged solely by verifier.c.
 *
 * Usage: cnf_cyclic m r > problem.cnf
 */
#include <stdio.h>
#include <stdlib.h>

static int m, r;
static int var(int i, int c) { return (i - 1) * r + c; }
static int *seedcol; /* 1-based colors, 0 = unseeded */

/* literal for "position i has color c", polarity-flipped so that the seed
 * assignment is the solver's default phase (all-negative under kissat). */
static void lit(int i, int c, int positive) {
  int flip = seedcol && seedcol[i] == c; /* seed-true vars are inverted */
  int sign = positive ^ flip ? 1 : -1;
  printf("%d ", sign * var(i, c));
}



int main(int argc, char **argv) {
  if (argc != 3 && argc != 4) {
    fprintf(stderr, "usage: %s m r [seedfile]\n", argv[0]);
    return 2;
  }
  m = atoi(argv[1]);
  r = atoi(argv[2]);
  if (argc == 4) {
    FILE *sf = fopen(argv[3], "r");
    if (!sf) { fprintf(stderr, "cannot open seed\n"); return 2; }
    seedcol = calloc((size_t)(m + 1), sizeof(int));
    char tok[1 << 15]; int stage = 0, sr = 0, ki = 0; long long sn = 0, idx = 0;
    while (fscanf(sf, "%32767s", tok) == 1) {
      if (tok[0] == '#') { int ch; while ((ch = fgetc(sf)) != EOF && ch != '\n') {} continue; }
      long long v = atoll(tok);
      if (stage == 0) { sr = (int)v; stage = 1; }
      else if (stage == 1) { if (++ki == sr) stage = 2; }
      else if (stage == 2) { sn = v; stage = 3; }
      else if (idx < sn && idx < m) { seedcol[++idx] = (int)v; }
      else idx++;
    }
    fclose(sf);
    if (sr != r) { fprintf(stderr, "seed r mismatch\n"); return 2; }
  }
  if (m < 3 || m % 2 == 0 || r < 2 || r > 64) {
    fprintf(stderr, "need odd m >= 3, r in [2,64]\n");
    return 2;
  }
  long long triples = (long long)m * ((m - 1) / 2);
  long long nclauses = m + (long long)m * r * (r - 1) / 2 + triples * r;
  if (!seedcol) {
    nclauses += 1 + (long long)m * (r - 1);
  }
  printf("p cnf %d %lld\n", m * r, nclauses);
  /* at least one color per position */
  for (int i = 1; i <= m; i++) {
    for (int c = 1; c <= r; c++) {
      lit(i, c, 1);
    }
    printf("0\n");
  }
  /* at most one color per position (pairwise; fine at r <= 17) */
  for (int i = 1; i <= m; i++) {
    for (int c1 = 1; c1 <= r; c1++) {
      for (int c2 = c1 + 1; c2 <= r; c2++) {
        lit(i, c1, 0);
        lit(i, c2, 0);
        printf("0\n");
      }
    }
  }
  /* no monochromatic cyclic 3-AP */
  for (int d = 1; d <= (m - 1) / 2; d++) {
    for (int a = 1; a <= m; a++) {
      int b = (a - 1 + d) % m + 1;
      int c2 = (a - 1 + 2 * d) % m + 1;
      for (int c = 1; c <= r; c++) {
        lit(a, c, 0);
        lit(b, c, 0);
        lit(c2, c, 0);
        printf("0\n");
      }
    }
  }
  /* symmetry: position 1 has color 1 (skip when phase-seeded: seed breaks it) */
  if (!seedcol) {
    printf("%d 0\n", var(1, 1));
    for (int i = 1; i <= m; i++) {
      for (int c = 2; c <= r; c++) {
        lit(i, c, 0);
        for (int j = 1; j < i; j++) {
          lit(j, c - 1, 1);
        }
        printf("0\n");
      }
    }
  }
  return 0;
}
