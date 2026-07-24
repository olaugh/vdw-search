/* verifier.c — independent van der Waerden certificate verifier.
 *
 * C11, no dependencies, shares no code with any generator or searcher.
 *
 * Usage: ./verifier certificate.txt
 *
 * Certificate format (whitespace-separated integer tokens; any line whose
 * first non-blank character is '#' is a comment and is ignored):
 *
 *   r                  number of colors, 1 <= r <= 255
 *   k_1 ... k_r        AP length forbidden in each color, k_i >= 2
 *   n                  size of the colored interval [1, n]
 *   c_1 ... c_n        colors, each in [1, r]; c_j is the color of integer j
 *
 * The verifier accepts iff for every color i there is NO arithmetic
 * progression a, a+d, ..., a+(k_i-1)d (d >= 1) inside [1, n] whose elements
 * are all colored i. Acceptance certifies the lower bound
 *
 *     w(r; k_1, ..., k_r) > n
 *
 * We use color-first notation throughout: for k_1 = ... = k_r = k this
 * is the classical W(r, k) > n.
 *
 * Exit codes: 0 = certificate VALID, 1 = INVALID (a monochromatic AP is
 * reported), 2 = malformed input.
 *
 * The scan enumerates every AP exactly once, identified by its first element
 * a and common difference d, and only tests the AP length assigned to the
 * color of a — any monochromatic AP is monochromatic in the color of its
 * first element, so nothing is missed.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { MAX_COLORS = 255 };
static const long long MAX_N = 500000000LL; /* 5e8: memory guard */
static const long long MAX_K = 1000000LL;

/* Reads the next integer token, skipping whitespace and '#' comment lines.
 * Returns 1 on success, 0 on end of input, dies on a malformed token. */
static int next_int(FILE *f, long long *out) {
  int ch;
  for (;;) {
    ch = fgetc(f);
    if (ch == EOF) {
      return 0;
    }
    if (ch == '#') {
      while (ch != EOF && ch != '\n') {
        ch = fgetc(f);
      }
      continue;
    }
    if (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r') {
      continue;
    }
    break;
  }
  int neg = 0;
  if (ch == '-') {
    neg = 1;
    ch = fgetc(f);
  }
  if (ch < '0' || ch > '9') {
    fprintf(stderr, "PARSE ERROR: unexpected character '%c'\n", ch);
    exit(2);
  }
  long long v = 0;
  while (ch >= '0' && ch <= '9') {
    v = v * 10 + (ch - '0');
    if (v > 4000000000000000000LL) {
      fprintf(stderr, "PARSE ERROR: integer token too large\n");
      exit(2);
    }
    ch = fgetc(f);
  }
  if (ch != EOF && ch != ' ' && ch != '\t' && ch != '\n' && ch != '\r' &&
      ch != '#') {
    fprintf(stderr, "PARSE ERROR: junk inside integer token\n");
    exit(2);
  }
  if (ch == '#') {
    while (ch != EOF && ch != '\n') {
      ch = fgetc(f);
    }
  }
  *out = neg ? -v : v;
  return 1;
}

static long long require_int(FILE *f, const char *what) {
  long long v;
  if (!next_int(f, &v)) {
    fprintf(stderr, "PARSE ERROR: unexpected end of input reading %s\n", what);
    exit(2);
  }
  return v;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "usage: %s certificate.txt\n", argv[0]);
    return 2;
  }
  FILE *f = fopen(argv[1], "r");
  if (!f) {
    fprintf(stderr, "PARSE ERROR: cannot open '%s'\n", argv[1]);
    return 2;
  }

  long long r = require_int(f, "r (number of colors)");
  if (r < 1 || r > MAX_COLORS) {
    fprintf(stderr, "PARSE ERROR: r=%lld out of range [1,%d]\n", r,
            MAX_COLORS);
    return 2;
  }
  long long klen[MAX_COLORS + 1];
  for (long long i = 1; i <= r; i++) {
    klen[i] = require_int(f, "k_i (AP length)");
    if (klen[i] < 2 || klen[i] > MAX_K) {
      fprintf(stderr, "PARSE ERROR: k_%lld=%lld out of range [2,%lld]\n", i,
              klen[i], MAX_K);
      return 2;
    }
  }
  long long n = require_int(f, "n (interval size)");
  if (n < 1 || n > MAX_N) {
    fprintf(stderr, "PARSE ERROR: n=%lld out of range [1,%lld]\n", n, MAX_N);
    return 2;
  }
  unsigned char *col = malloc((size_t)(n + 1));
  if (!col) {
    fprintf(stderr, "PARSE ERROR: out of memory for n=%lld\n", n);
    return 2;
  }
  for (long long j = 1; j <= n; j++) {
    long long c = require_int(f, "color");
    if (c < 1 || c > r) {
      fprintf(stderr,
              "PARSE ERROR: color of element %lld is %lld, outside [1,%lld]\n",
              j, c, r);
      return 2;
    }
    col[j] = (unsigned char)c;
  }
  long long extra;
  if (next_int(f, &extra)) {
    fprintf(stderr, "PARSE ERROR: trailing token %lld after %lld colors\n",
            extra, n);
    return 2;
  }
  fclose(f);

  /* Exhaustive scan: every AP, keyed by first element and difference. */
  for (long long a = 1; a <= n; a++) {
    const unsigned char c = col[a];
    const long long k = klen[c];
    const long long maxd = (n - a) / (k - 1);
    for (long long d = 1; d <= maxd; d++) {
      long long x = a + d;
      int mono = 1;
      for (long long j = 1; j < k; j++, x += d) {
        if (col[x] != c) {
          mono = 0;
          break;
        }
      }
      if (mono) {
        printf("INVALID: monochromatic AP in color %d (forbidden length "
               "%lld): start=%lld diff=%lld elements=",
               (int)c, k, a, d);
        for (long long j = 0; j < k; j++) {
          printf("%lld%s", a + j * d, j + 1 < k ? "," : "\n");
        }
        free(col);
        return 1;
      }
    }
  }

  printf("VALID: n=%lld; colors=%lld; forbidden AP lengths=(", n, r);
  for (long long i = 1; i <= r; i++) {
    printf("%lld%s", klen[i], i < r ? "," : "");
  }
  printf("); color-first notation: ");
  int uniform = 1;
  for (long long i = 2; i <= r; i++) {
    if (klen[i] != klen[1]) {
      uniform = 0;
      break;
    }
  }
  if (uniform) {
    printf("W(%lld,%lld) > %lld\n", r, klen[1], n);
  } else {
    printf("w(%lld;", r);
    for (long long i = 1; i <= r; i++) {
      printf("%lld%s", klen[i], i < r ? "," : "");
    }
    printf(") > %lld\n", n);
  }
  free(col);
  return 0;
}
