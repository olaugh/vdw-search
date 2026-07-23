/* cnf_linear.c — DIMACS emitter: does an r-coloring of [1,n] with no
 * monochromatic 3-term AP exist? Decision instance for W(r,3) > n.
 * Vars x_{i,c} = (i-1)*r + c. Symmetry breaking: first-occurrence color
 * ordering (color c can first appear only after color c-1 has appeared),
 * which is solution-preserving up to color relabeling.
 * Generator-side tooling; SAT models are decoded and judged by verifier.c.
 *
 * Usage: cnf_linear n r > problem.cnf
 */
#include <stdio.h>
#include <stdlib.h>

static int n, r;
static int *seedcol; /* 1-based colors, 0 = unseeded */
static int var(int i, int c) { return (i - 1) * r + c; }
/* literal for "position i has color c", polarity-flipped so that the seed
 * assignment is the solver's default phase (all-negative under kissat). */
static void lit(int i, int c, int positive) {
  int flip = seedcol && seedcol[i] == c; /* seed-true vars are inverted */
  int sign = positive ^ flip ? 1 : -1;
  printf("%d ", sign * var(i, c));
}

int main(int argc, char **argv) {
  if (argc != 3 && argc != 4) {
    fprintf(stderr, "usage: %s n r [seedfile]\n", argv[0]);
    return 2;
  }
  n = atoi(argv[1]);
  r = atoi(argv[2]);
  if (argc == 4) {
    FILE *f = fopen(argv[3], "r");
    if (!f) {
      fprintf(stderr, "cannot open seed\n");
      return 2;
    }
    seedcol = calloc((size_t)(n + 1), sizeof(int));
    char tok[1 << 15];
    int stage = 0, sr = 0, ki = 0;
    long long sn = 0, idx = 0;
    while (fscanf(f, "%32767s", tok) == 1) {
      if (tok[0] == '#') {
        int ch;
        while ((ch = fgetc(f)) != EOF && ch != '\n') {
        }
        continue;
      }
      long long v = atoll(tok);
      if (stage == 0) {
        sr = (int)v;
        stage = 1;
      } else if (stage == 1) {
        if (++ki == sr) {
          stage = 2;
        }
      } else if (stage == 2) {
        sn = v;
        stage = 3;
      } else if (idx < sn && idx < n) {
        seedcol[++idx] = (int)v;
      } else {
        idx++;
      }
    }
    fclose(f);
    if (sr != r) {
      fprintf(stderr, "seed r mismatch\n");
      return 2;
    }
  }
  if (n < 3 || r < 2 || r > 64) {
    fprintf(stderr, "need n >= 3, r in [2,64]\n");
    return 2;
  }
  long long triples = 0;
  for (int d = 1; 2 * d < n; d++) {
    triples += n - 2 * d;
  }
  long long nclauses = n + (long long)n * r * (r - 1) / 2 + triples * r;
  if (!seedcol) {
    nclauses += (long long)n * (r - 1);
  }
  printf("p cnf %d %lld\n", n * r, nclauses);
  for (int i = 1; i <= n; i++) {
    for (int c = 1; c <= r; c++) {
      lit(i, c, 1);
    }
    printf("0\n");
  }
  for (int i = 1; i <= n; i++) {
    for (int c1 = 1; c1 <= r; c1++) {
      for (int c2 = c1 + 1; c2 <= r; c2++) {
        lit(i, c1, 0);
        lit(i, c2, 0);
        printf("0\n");
      }
    }
  }
  for (int d = 1; 2 * d < n; d++) {
    for (int a = 1; a + 2 * d <= n; a++) {
      for (int c = 1; c <= r; c++) {
        lit(a, c, 0);
        lit(a + d, c, 0);
        lit(a + 2 * d, c, 0);
        printf("0\n");
      }
    }
  }
  /* first-occurrence ordering only when NOT phase-seeding (a fixed seed
   * already breaks color symmetry; the ordering clauses would fight it) */
  if (!seedcol) {
    for (int i = 1; i <= n; i++) {
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
