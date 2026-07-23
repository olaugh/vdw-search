/* sls.c — stochastic local search for r-colorings of [1,n] with no
 * monochromatic 3-term AP (k=3 uniform; the T4/Komkov-family searcher).
 *
 * GENERATOR-SIDE code: shares nothing with verifier.c; any zero-violation
 * output must pass the independent verifier before it is believed.
 *
 * Design per LOG.md Phase 3 note: WalkSAT/SKC baseline + DDFW, violated-AP
 * list with position index, per-element CSR adjacency, tabu, adaptive noise,
 * DDFW weight transfer at local minima, seed/perturb/reverse/cold starts.
 *
 * Usage:
 *   sls -n N -r R -m walksat|ddfw -f MAXFLIPS -s RNGSEED
 *       [-i seedfile] [-P perturb_percent] [-R] -o outfile
 * Exit 0: zero violations reached (certificate written to outfile).
 * Exit 1: flip budget exhausted (stats on stderr).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---------- xorshift RNG (seedable, fast) ---------- */
static unsigned long long rng_state;
static unsigned long long rng_next(void) {
  unsigned long long x = rng_state;
  x ^= x << 13;
  x ^= x >> 7;
  x ^= x << 17;
  rng_state = x;
  return x;
}
static unsigned rng_below(unsigned m) { return (unsigned)(rng_next() % m); }

/* ---------- problem state ---------- */
static int n, r;
static unsigned char *col;    /* col[1..n], colors 0..r-1 */
static int nap;               /* number of APs */
static int *ap_a, *ap_d;      /* AP i = (a, a+d, a+2d) */
static int *w;                /* DDFW weights */
static int *adj, *adj_start;  /* CSR: element -> AP indices */
static int *vlist, *vpos, nv; /* violated APs, position index */
static long long *tabu_until; /* per-element tabu expiry (flip count) */

static int cyclic_mode;
static int wrap(int x) {
  if (!cyclic_mode) {
    return x;
  }
  int m = n;
  x = (x - 1) % m;
  return x + 1;
}
static int ap_violated(int i) {
  int a = ap_a[i], d = ap_d[i];
  return col[a] == col[wrap(a + d)] && col[a] == col[wrap(a + 2 * d)];
}

static void v_add(int i) {
  if (vpos[i] < 0) {
    vpos[i] = nv;
    vlist[nv++] = i;
  }
}
static void v_del(int i) {
  int pos = vpos[i];
  if (pos >= 0) {
    int last = vlist[--nv];
    vlist[pos] = last;
    vpos[last] = pos;
    vpos[i] = -1;
  }
}

/* Recolor element e to color c, updating the violated list. */
static void recolor(int e, int c) {
  col[e] = (unsigned char)c;
  for (int t = adj_start[e]; t < adj_start[e + 1]; t++) {
    int i = adj[t];
    if (ap_violated(i)) {
      v_add(i);
    } else {
      v_del(i);
    }
  }
}

/* Weighted break of recoloring e -> c: sum of weights of APs through e that
 * become violated minus those that become satisfied. */
static long long delta_for(int e, int c) {
  int old = col[e];
  long long delta = 0;
  col[e] = (unsigned char)c;
  for (int t = adj_start[e]; t < adj_start[e + 1]; t++) {
    int i = adj[t];
    int now = ap_violated(i);
    int was = vpos[i] >= 0;
    if (now && !was) {
      delta += w[i];
    } else if (!now && was) {
      delta -= w[i];
    }
  }
  col[e] = (unsigned char)old;
  return delta;
}

int main(int argc, char **argv) {
  long long maxflips = 200000000LL;
  unsigned long long seed = 12345;
  const char *mode = "ddfw", *seedfile = NULL, *outfile = NULL;
  int perturb_pct = 0, reverse_seed = 0, cyclic = 0;
  n = 0;
  r = 0;
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-n")) {
      n = atoi(argv[++i]);
    } else if (!strcmp(argv[i], "-r")) {
      r = atoi(argv[++i]);
    } else if (!strcmp(argv[i], "-m")) {
      mode = argv[++i];
    } else if (!strcmp(argv[i], "-f")) {
      maxflips = atoll(argv[++i]);
    } else if (!strcmp(argv[i], "-s")) {
      seed = (unsigned long long)atoll(argv[++i]);
    } else if (!strcmp(argv[i], "-i")) {
      seedfile = argv[++i];
    } else if (!strcmp(argv[i], "-P")) {
      perturb_pct = atoi(argv[++i]);
    } else if (!strcmp(argv[i], "-R")) {
      reverse_seed = 1;
    } else if (!strcmp(argv[i], "-C")) {
      cyclic = 1;
    } else if (!strcmp(argv[i], "-o")) {
      outfile = argv[++i];
    } else {
      fprintf(stderr, "unknown arg %s\n", argv[i]);
      return 2;
    }
  }
  if (n < 3 || r < 2 || r > 32 || !outfile) {
    fprintf(stderr, "need -n>=3 -r in [2,32] -o outfile\n");
    return 2;
  }
  if (cyclic && n % 2 == 0) {
    fprintf(stderr, "cyclic mode requires odd n (see unroll analysis)\n");
    return 2;
  }
  int ddfw = !strcmp(mode, "ddfw");
  cyclic_mode = cyclic;
  rng_state = seed * 2654435761ULL + 1;

  /* ---- initial coloring: cold random, optionally overlaid by a seed ---- */
  col = malloc((size_t)(n + 1));
  for (int e = 1; e <= n; e++) {
    col[e] = (unsigned char)rng_below((unsigned)r);
  }
  if (seedfile) {
    FILE *f = fopen(seedfile, "r");
    if (!f) {
      fprintf(stderr, "cannot open seed %s\n", seedfile);
      return 2;
    }
    /* cert format: comments, r, k*r, n', colors (1-based) */
    char line[1 << 16];
    long long vals[4] = {0};
    int nv_read = 0;
    int sr = 0;
    long long sn = 0;
    int *tmp = NULL, ti = 0;
    while (fscanf(f, "%65535s", line) == 1) {
      if (line[0] == '#') {
        int ch;
        while ((ch = fgetc(f)) != EOF && ch != '\n') {
        }
        continue;
      }
      long long v = atoll(line);
      if (nv_read == 0) {
        sr = (int)v;
        nv_read++;
      } else if (nv_read <= sr) {
        nv_read++; /* skip k_i */
      } else if (nv_read == sr + 1) {
        sn = v;
        tmp = malloc((size_t)(sn + 1) * sizeof(int));
        nv_read++;
      } else if (ti < sn) {
        tmp[++ti] = (int)v - 1; /* to 0-based */
      }
      (void)vals;
    }
    fclose(f);
    if (sr != r || ti != sn || sn > n) {
      fprintf(stderr, "seed mismatch (r=%d vs %d, read %d of %lld, n=%d)\n",
              sr, r, ti, sn, n);
      return 2;
    }
    for (int e = 1; e <= sn; e++) {
      col[e] = (unsigned char)(reverse_seed ? tmp[sn + 1 - e] : tmp[e]);
    }
    free(tmp);
  }
  for (int e = 1; e <= n; e++) {
    if (perturb_pct > 0 && (int)rng_below(100) < perturb_pct) {
      col[e] = (unsigned char)rng_below((unsigned)r);
    }
  }

  /* ---- enumerate APs and build CSR adjacency ---- */
  nap = 0;
  if (cyclic) {
    nap = n * ((n - 1) / 2);
  } else {
    for (int d = 1; 2 * d < n; d++) {
      nap += n - 2 * d;
    }
  }
  ap_a = malloc((size_t)nap * sizeof(int));
  ap_d = malloc((size_t)nap * sizeof(int));
  w = malloc((size_t)nap * sizeof(int));
  int idx = 0;
  if (cyclic) {
    for (int d = 1; d <= (n - 1) / 2; d++) {
      for (int a = 1; a <= n; a++) {
        ap_a[idx] = a;
        ap_d[idx] = d;
        w[idx] = 8;
        idx++;
      }
    }
  } else {
    for (int d = 1; 2 * d < n; d++) {
      for (int a = 1; a + 2 * d <= n; a++) {
        ap_a[idx] = a;
        ap_d[idx] = d;
        w[idx] = 8;
        idx++;
      }
    }
  }
  adj_start = calloc((size_t)(n + 2), sizeof(int));
  for (int i = 0; i < nap; i++) {
    adj_start[ap_a[i] + 1]++;
    adj_start[wrap(ap_a[i] + ap_d[i]) + 1]++;
    adj_start[wrap(ap_a[i] + 2 * ap_d[i]) + 1]++;
  }
  for (int e = 1; e <= n + 1; e++) {
    adj_start[e] += adj_start[e - 1];
  }
  adj = malloc((size_t)adj_start[n + 1] * sizeof(int));
  int *fill = malloc((size_t)(n + 2) * sizeof(int));
  memcpy(fill, adj_start, (size_t)(n + 2) * sizeof(int));
  for (int i = 0; i < nap; i++) {
    adj[fill[ap_a[i]]++] = i;
    adj[fill[wrap(ap_a[i] + ap_d[i])]++] = i;
    adj[fill[wrap(ap_a[i] + 2 * ap_d[i])]++] = i;
  }
  free(fill);

  vlist = malloc((size_t)nap * sizeof(int));
  vpos = malloc((size_t)nap * sizeof(int));
  nv = 0;
  for (int i = 0; i < nap; i++) {
    vpos[i] = -1;
    if (ap_violated(i)) {
      v_add(i);
    }
  }
  tabu_until = calloc((size_t)(n + 1), sizeof(long long));

  /* ---- search loop ---- */
  double noise = 0.25;
  const int TABU_TENURE = 12;
  int best_nv = nv;
  long long last_improve = 0;
  long long flips = 0;
  while (nv > 0 && flips < maxflips) {
    flips++;
    /* pick a violated AP: uniform (walksat) / weight-biased best-of-3 */
    int ap = vlist[rng_below((unsigned)nv)];
    if (ddfw) {
      for (int trial = 0; trial < 2; trial++) {
        int cand = vlist[rng_below((unsigned)nv)];
        if (w[cand] > w[ap]) {
          ap = cand;
        }
      }
    }
    int elems[3] = {ap_a[ap], wrap(ap_a[ap] + ap_d[ap]),
                    wrap(ap_a[ap] + 2 * ap_d[ap])};
    int be = -1, bc = -1;
    long long bd = 0;
    if ((double)rng_below(1000000) / 1e6 < noise) {
      be = elems[rng_below(3)];
      do {
        bc = (int)rng_below((unsigned)r);
      } while (bc == col[be]);
    } else {
      long long best = 0;
      int found = 0;
      for (int ei = 0; ei < 3; ei++) {
        int e = elems[ei];
        if (tabu_until[e] > flips) {
          continue;
        }
        for (int c = 0; c < r; c++) {
          if (c == col[e]) {
            continue;
          }
          long long dl = delta_for(e, c);
          if (!found || dl < best) {
            best = dl;
            be = e;
            bc = c;
            found = 1;
          }
        }
      }
      if (!found) { /* all tabu: random move */
        be = elems[rng_below(3)];
        do {
          bc = (int)rng_below((unsigned)r);
        } while (bc == col[be]);
      }
      bd = best;
      if (ddfw && found && bd > 0) {
        /* local minimum: transfer weight from a satisfied AP sharing an
         * element (heaviest, or random 1% of the time), then continue. */
        int donor = -1;
        int pick_random = rng_below(100) == 0;
        for (int ei = 0; ei < 3 && donor < 0; ei++) {
          int e = elems[ei];
          int start = adj_start[e], end = adj_start[e + 1];
          int bestw = -1;
          for (int t = start; t < end; t++) {
            int i = adj[t];
            if (vpos[i] < 0 && i != ap) {
              if (pick_random) {
                if (rng_below(8) == 0) {
                  donor = i;
                  break;
                }
              } else if (w[i] > bestw) {
                bestw = w[i];
                donor = i;
              }
            }
          }
        }
        if (donor >= 0 && w[donor] >= 2) {
          w[donor] -= 2;
          w[ap] += 2;
        }
      }
    }
    recolor(be, bc);
    tabu_until[be] = flips + TABU_TENURE;
    if (nv < best_nv) {
      best_nv = nv;
      last_improve = flips;
      noise = 0.25 + (noise - 0.25) * 0.5;
    } else if (flips - last_improve > 1000000) {
      noise = noise * 1.2;
      if (noise > 0.6) {
        noise = 0.6;
      }
      last_improve = flips; /* reset stagnation clock */
    }
  }

  if (nv == 0) {
    FILE *f = fopen(outfile, "w");
    fprintf(f, "# sls %s: n=%d r=%d mode=%s rngseed=%llu flips=%lld"
               " seed=%s P=%d R=%d\n",
            cyclic ? "CYCLIC-CORE (unroll before verifying)" : "certificate",
            n, r, mode, seed, flips, seedfile ? seedfile : "-", perturb_pct,
            reverse_seed);
    fprintf(f, "%d\n", r);
    for (int i = 0; i < r; i++) {
      fprintf(f, "3%s", i + 1 < r ? " " : "\n");
    }
    fprintf(f, "%d\n", n);
    for (int e = 1; e <= n; e++) {
      fprintf(f, "%d%s", col[e] + 1, e % 60 == 0 || e == n ? "\n" : " ");
    }
    fclose(f);
    fprintf(stderr, "SOLVED n=%d r=%d flips=%lld\n", n, r, flips);
    return 0;
  }
  fprintf(stderr, "exhausted: n=%d r=%d flips=%lld best_nv=%d final_nv=%d\n",
          n, r, flips, best_nv, nv);
  return 1;
}
