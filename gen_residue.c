/* gen_residue.c — power-residue (Rabung) + cyclic-zipper certificate
 * GENERATOR. Deliberately shares no code with verifier.c; its output is
 * only ever trusted after verifier.c accepts it.
 *
 * Construction (Rabung 1979; Herwig–Heule–van Lambalgen–van Maaren 2007):
 *  1. Base cyclic coloring of length p (p prime, r | p-1): position x in
 *     [2, p] gets color (ind_rho(x-1) mod r) + 1 where rho is a primitive
 *     root mod p; position 1 (residue 0) is a free choice.
 *  2. Zip z times (z >= 0): spread to odd positions, flip colors
 *     (i -> r+1-i), shift by the old length, merge. Length doubles.
 *  3. Repetitive expansion: repeat the cyclic coloring (k-1) times and
 *     append one extra number. Bound: W(r,k) > (k-1) * p * 2^z + 1.
 *
 * Free choices (colors not fixed by the construction) are driven by the
 * `choice` argument, interpreted base-r, lowest digit first:
 *   z = 0: the k residue-0 positions (1, p+1, ..., (k-1)p+1) in order.
 *   z >= 1: base position 1, then the final appended position.
 * The driver scans choices; verifier.c decides which are certificates.
 *
 * Usage: gen_residue p r k z choice > cert.txt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static long long modmul(long long a, long long b, long long m) {
  return (long long)((__int128)a * b % m);
}

static long long modpow(long long b, long long e, long long m) {
  long long acc = 1 % m;
  b %= m;
  while (e > 0) {
    if (e & 1) {
      acc = modmul(acc, b, m);
    }
    b = modmul(b, b, m);
    e >>= 1;
  }
  return acc;
}

static int is_prime(long long p) {
  if (p < 2) {
    return 0;
  }
  for (long long d = 2; d * d <= p; d++) {
    if (p % d == 0) {
      return 0;
    }
  }
  return 1;
}

/* Smallest primitive root mod p (p small enough for trial division). */
static long long primitive_root(long long p) {
  long long phi = p - 1;
  long long factors[64];
  int nf = 0;
  long long m = phi;
  for (long long d = 2; d * d <= m; d++) {
    if (m % d == 0) {
      factors[nf++] = d;
      while (m % d == 0) {
        m /= d;
      }
    }
  }
  if (m > 1) {
    factors[nf++] = m;
  }
  for (long long g = 2; g < p; g++) {
    int ok = 1;
    for (int i = 0; i < nf; i++) {
      if (modpow(g, phi / factors[i], p) == 1) {
        ok = 0;
        break;
      }
    }
    if (ok) {
      return g;
    }
  }
  return -1;
}

int main(int argc, char **argv) {
  if (argc != 6) {
    fprintf(stderr, "usage: %s p r k z choice\n", argv[0]);
    return 2;
  }
  long long p = atoll(argv[1]);
  long long r = atoll(argv[2]);
  long long k = atoll(argv[3]);
  long long z = atoll(argv[4]);
  long long choice = atoll(argv[5]);
  if (!is_prime(p) || r < 2 || (p - 1) % r != 0 || k < 3 || z < 0 || z > 4) {
    fprintf(stderr, "bad parameters (need p prime, r|p-1, k>=3, 0<=z<=4)\n");
    return 2;
  }

  /* Discrete-log table via primitive-root enumeration (fine at gen scale). */
  long long rho = primitive_root(p);
  int *ind = malloc((size_t)p * sizeof(int));
  if (!ind) {
    return 2;
  }
  long long a = 1;
  for (long long i = 0; i < p - 1; i++) {
    ind[a] = (int)(i % r);
    a = modmul(a, rho, p);
  }

  long long n = p;
  int *cyc = malloc((size_t)(p << (z + 1)) * sizeof(int));
  if (!cyc) {
    return 2;
  }
  /* Base cyclic coloring, 1-based positions 1..p. Position 1 = residue 0. */
  long long ch = choice;
  int pos1_color;
  if (z >= 1) {
    pos1_color = (int)(ch % r) + 1;
    ch /= r;
  } else {
    pos1_color = 1; /* placeholder; z=0 assigns all residue-0 slots later */
  }
  cyc[1] = pos1_color;
  for (long long x = 2; x <= p; x++) {
    cyc[x] = ind[x - 1] + 1;
  }

  /* Zip z times: spread/turn/shift/merge (Herwig et al. section 4.3). */
  for (long long zi = 0; zi < z; zi++) {
    int *nz = malloc((size_t)(2 * n + 1) * sizeof(int));
    if (!nz) {
      return 2;
    }
    memset(nz, 0, (size_t)(2 * n + 1) * sizeof(int));
    for (long long j = 1; j <= n; j++) {
      long long spread = 2 * j - 1;
      nz[spread] = cyc[j];                                /* spreading  */
      /* Herwig et al.: the shift is by the ORIGINAL base length p at every
       * zip level ("using the original certificate length to shift" for
       * second-degree zips), not by the current length n. */
      long long shifted = ((spread - 1 - p) % (2 * n) + (2 * n)) % (2 * n) + 1;
      nz[shifted] = (int)(r + 1) - cyc[j];                /* turn+shift */
    }
    for (long long j = 1; j <= 2 * n; j++) {
      if (nz[j] == 0) {
        fprintf(stderr, "zip left position %lld uncovered\n", j);
        return 2;
      }
    }
    memcpy(cyc + 1, nz + 1, (size_t)(2 * n) * sizeof(int));
    free(nz);
    n *= 2;
  }

  /* Repetitive expansion to (k-1)*n + 1. */
  long long total = (k - 1) * n + 1;
  int *out = malloc((size_t)(total + 1) * sizeof(int));
  if (!out) {
    return 2;
  }
  for (long long y = 1; y <= total; y++) {
    out[y] = cyc[(y - 1) % n + 1];
  }
  if (z == 0) {
    /* The k residue-0 positions are free (Rabung: "not all one color"). */
    for (long long t = 0; t < k; t++) {
      out[t * p + 1] = (int)(ch % r) + 1;
      ch /= r;
    }
  } else {
    out[total] = (int)(ch % r) + 1; /* final appended number */
    ch /= r;
  }

  printf("# power-residue certificate: p=%lld rho=%lld r=%lld k=%lld z=%lld "
         "choice=%lld\n",
         p, rho, r, k, z, choice);
  printf("# claims W(%lld,%lld) > %lld\n", r, k, total);
  printf("%lld\n", r);
  for (long long i = 0; i < r; i++) {
    printf("%lld%s", k, i + 1 < r ? " " : "\n");
  }
  printf("%lld\n", total);
  for (long long y = 1; y <= total; y++) {
    printf("%d%s", out[y], y % 60 == 0 || y == total ? "\n" : " ");
  }
  free(out);
  free(cyc);
  free(ind);
  return 0;
}
