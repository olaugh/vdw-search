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

int main(int argc, char **argv) {
  if (argc != 3) {
    fprintf(stderr, "usage: %s m r\n", argv[0]);
    return 2;
  }
  m = atoi(argv[1]);
  r = atoi(argv[2]);
  if (m < 3 || m % 2 == 0 || r < 2 || r > 64) {
    fprintf(stderr, "need odd m >= 3, r in [2,64]\n");
    return 2;
  }
  long long triples = (long long)m * ((m - 1) / 2);
  long long nclauses = m                          /* ALO  */
                       + (long long)m * r * (r - 1) / 2 /* AMO */
                       + triples * r              /* AP   */
                       + 1;                       /* fix  */
  printf("p cnf %d %lld\n", m * r, nclauses);
  /* at least one color per position */
  for (int i = 1; i <= m; i++) {
    for (int c = 1; c <= r; c++) {
      printf("%d ", var(i, c));
    }
    printf("0\n");
  }
  /* at most one color per position (pairwise; fine at r <= 17) */
  for (int i = 1; i <= m; i++) {
    for (int c1 = 1; c1 <= r; c1++) {
      for (int c2 = c1 + 1; c2 <= r; c2++) {
        printf("-%d -%d 0\n", var(i, c1), var(i, c2));
      }
    }
  }
  /* no monochromatic cyclic 3-AP */
  for (int d = 1; d <= (m - 1) / 2; d++) {
    for (int a = 1; a <= m; a++) {
      int b = (a - 1 + d) % m + 1;
      int c2 = (a - 1 + 2 * d) % m + 1;
      for (int c = 1; c <= r; c++) {
        printf("-%d -%d -%d 0\n", var(a, c), var(b, c), var(c2, c));
      }
    }
  }
  /* symmetry: position 1 has color 1 */
  printf("%d 0\n", var(1, 1));
  return 0;
}
