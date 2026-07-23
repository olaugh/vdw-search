/* t2_sls.c — direct stochastic local search for mixed w(2;3,t).
 *
 * GENERATOR-SIDE C11 code. This file shares no code with verifier.c or the
 * M2/T4 searcher. A zero maintained violation count is only a candidate:
 * the standalone verifier must accept the emitted certificate.
 *
 * Boolean convention:
 *   bit 1 = certificate color 1, which must avoid 3-term APs
 *   bit 0 = certificate color 2, which must avoid t-term APs
 *
 * Thus each 3-AP is an implicit negative clause and each t-AP an implicit
 * positive clause. Constraints, true counts, per-element CSR incidence, and
 * an O(1) violated-constraint list are maintained explicitly.
 *
 * Usage:
 *   t2_sls -n N -t T -m walksat|ddfw -f MAXFLIPS -s SEED -o CANDIDATE
 *          [-c random|seed|cyclic|palindrome|digit] [-i seed-cert]
 *          [-P perturb-percent] [-R] [-q template-parameter]
 *          [-D color1-density-percent] [-l seconds] [-p progress-flips]
 *   t2_sls --self-test
 *
 * Exit 0: maintained zero violations and candidate written.
 * Exit 1: flip/time budget exhausted.
 * Exit 2: bad input or allocation failure.
 */

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

enum { INITIAL_WEIGHT = 8, TABU_TENURE = 12 };

typedef enum ConstraintKind {
  CONSTRAINT_NEGATIVE_3,
  CONSTRAINT_POSITIVE_T,
} ConstraintKind;

typedef enum StartClass {
  START_RANDOM,
  START_SEED,
  START_CYCLIC,
  START_PALINDROME,
  START_DIGIT,
} StartClass;

typedef struct Constraint {
  int start;
  int diff;
  int length;
  int true_count;
  int weight;
  ConstraintKind kind;
} Constraint;

typedef struct Problem {
  int n;
  int t;
  int num_constraints;
  Constraint *constraints;
  unsigned char *bits; /* 1..n */
  size_t *adj_start;   /* 1..n+1 */
  int *adj;
  int *violated;
  int *violated_pos;
  int num_violated;
  long long *tabu_until;
} Problem;

typedef struct Delta {
  long long weighted;
  int unweighted;
} Delta;

static uint64_t rng_state;

static void die(const char *message) {
  fprintf(stderr, "error: %s\n", message);
  exit(2);
}

static void *xmalloc(size_t bytes) {
  void *p = malloc(bytes);
  if (p == NULL && bytes != 0) {
    die("out of memory");
  }
  return p;
}

static void *xcalloc(size_t count, size_t bytes) {
  if (bytes != 0 && count > SIZE_MAX / bytes) {
    die("allocation size overflow");
  }
  void *p = calloc(count, bytes);
  if (p == NULL && count != 0 && bytes != 0) {
    die("out of memory");
  }
  return p;
}

static uint64_t rng_next(void) {
  uint64_t x = rng_state;
  x ^= x << 13;
  x ^= x >> 7;
  x ^= x << 17;
  rng_state = x;
  return x;
}

static unsigned rng_below(unsigned bound) {
  if (bound == 0) {
    die("internal RNG bound is zero");
  }
  return (unsigned)(rng_next() % bound);
}

static double rng_unit(void) {
  return (double)(rng_next() >> 11) * (1.0 / 9007199254740992.0);
}

static long long parse_ll(const char *text, const char *what) {
  char *end = NULL;
  errno = 0;
  long long value = strtoll(text, &end, 10);
  if (errno != 0 || end == text || *end != '\0') {
    fprintf(stderr, "error: invalid %s: %s\n", what, text);
    exit(2);
  }
  return value;
}

static uint64_t parse_u64(const char *text, const char *what) {
  char *end = NULL;
  errno = 0;
  unsigned long long value = strtoull(text, &end, 10);
  if (errno != 0 || end == text || *end != '\0') {
    fprintf(stderr, "error: invalid %s: %s\n", what, text);
    exit(2);
  }
  return (uint64_t)value;
}

static bool constraint_is_violated_count(const Constraint *constraint,
                                         int true_count) {
  if (constraint->kind == CONSTRAINT_NEGATIVE_3) {
    return true_count == constraint->length;
  }
  return true_count == 0;
}

static bool constraint_is_violated(const Constraint *constraint) {
  return constraint_is_violated_count(constraint, constraint->true_count);
}

static void violated_add(Problem *problem, int constraint_index) {
  if (problem->violated_pos[constraint_index] >= 0) {
    return;
  }
  problem->violated_pos[constraint_index] = problem->num_violated;
  problem->violated[problem->num_violated++] = constraint_index;
}

static void violated_remove(Problem *problem, int constraint_index) {
  const int position = problem->violated_pos[constraint_index];
  if (position < 0) {
    return;
  }
  const int last = problem->violated[--problem->num_violated];
  problem->violated[position] = last;
  problem->violated_pos[last] = position;
  problem->violated_pos[constraint_index] = -1;
}

static long long progression_count(int n, int length) {
  const int span = length - 1;
  long long count = 0;
  for (int diff = 1; (long long)span * diff < n; diff++) {
    count += n - span * diff;
  }
  return count;
}

static void fill_constraint(Constraint *constraint, int start, int diff,
                            int length, ConstraintKind kind) {
  constraint->start = start;
  constraint->diff = diff;
  constraint->length = length;
  constraint->true_count = 0;
  constraint->weight = INITIAL_WEIGHT;
  constraint->kind = kind;
}

static void problem_build(Problem *problem, int n, int t) {
  memset(problem, 0, sizeof(*problem));
  problem->n = n;
  problem->t = t;
  const long long num_3 = progression_count(n, 3);
  const long long num_t = progression_count(n, t);
  const long long total = num_3 + num_t;
  if (total <= 0 || total > INT_MAX) {
    die("constraint count is outside the supported range");
  }
  problem->num_constraints = (int)total;
  problem->constraints =
      xmalloc((size_t)problem->num_constraints * sizeof(Constraint));
  problem->bits = xmalloc((size_t)(n + 1));
  problem->violated =
      xmalloc((size_t)problem->num_constraints * sizeof(int));
  problem->violated_pos =
      xmalloc((size_t)problem->num_constraints * sizeof(int));
  problem->tabu_until = xcalloc((size_t)(n + 1), sizeof(long long));

  int index = 0;
  for (int diff = 1; 2LL * diff < n; diff++) {
    for (int start = 1; start + 2 * diff <= n; start++) {
      fill_constraint(&problem->constraints[index++], start, diff, 3,
                      CONSTRAINT_NEGATIVE_3);
    }
  }
  const int tspan = t - 1;
  for (int diff = 1; (long long)tspan * diff < n; diff++) {
    for (int start = 1; start + tspan * diff <= n; start++) {
      fill_constraint(&problem->constraints[index++], start, diff, t,
                      CONSTRAINT_POSITIVE_T);
    }
  }
  if (index != problem->num_constraints) {
    die("internal constraint enumeration mismatch");
  }

  problem->adj_start = xcalloc((size_t)(n + 2), sizeof(size_t));
  for (int ci = 0; ci < problem->num_constraints; ci++) {
    const Constraint *constraint = &problem->constraints[ci];
    int element = constraint->start;
    for (int j = 0; j < constraint->length;
         j++, element += constraint->diff) {
      problem->adj_start[element + 1]++;
    }
  }
  for (int element = 1; element <= n + 1; element++) {
    if (problem->adj_start[element] >
        SIZE_MAX - problem->adj_start[element - 1]) {
      die("adjacency size overflow");
    }
    problem->adj_start[element] += problem->adj_start[element - 1];
  }
  problem->adj =
      xmalloc(problem->adj_start[n + 1] * sizeof(*problem->adj));
  size_t *fill = xmalloc((size_t)(n + 2) * sizeof(*fill));
  memcpy(fill, problem->adj_start, (size_t)(n + 2) * sizeof(*fill));
  for (int ci = 0; ci < problem->num_constraints; ci++) {
    const Constraint *constraint = &problem->constraints[ci];
    int element = constraint->start;
    for (int j = 0; j < constraint->length;
         j++, element += constraint->diff) {
      problem->adj[fill[element]++] = ci;
    }
  }
  free(fill);
}

static void problem_destroy(Problem *problem) {
  free(problem->constraints);
  free(problem->bits);
  free(problem->adj_start);
  free(problem->adj);
  free(problem->violated);
  free(problem->violated_pos);
  free(problem->tabu_until);
  memset(problem, 0, sizeof(*problem));
}

static void problem_reset_counts(Problem *problem) {
  problem->num_violated = 0;
  memset(problem->tabu_until, 0,
         (size_t)(problem->n + 1) * sizeof(*problem->tabu_until));
  for (int ci = 0; ci < problem->num_constraints; ci++) {
    Constraint *constraint = &problem->constraints[ci];
    constraint->true_count = 0;
    constraint->weight = INITIAL_WEIGHT;
    int element = constraint->start;
    for (int j = 0; j < constraint->length;
         j++, element += constraint->diff) {
      constraint->true_count += problem->bits[element] != 0;
    }
    problem->violated_pos[ci] = -1;
    if (constraint_is_violated(constraint)) {
      violated_add(problem, ci);
    }
  }
}

static Delta delta_for_flip(const Problem *problem, int element) {
  Delta delta = {0, 0};
  const int true_delta = problem->bits[element] ? -1 : 1;
  for (size_t p = problem->adj_start[element];
       p < problem->adj_start[element + 1]; p++) {
    const int ci = problem->adj[p];
    const Constraint *constraint = &problem->constraints[ci];
    const bool was = problem->violated_pos[ci] >= 0;
    const bool now = constraint_is_violated_count(
        constraint, constraint->true_count + true_delta);
    if (now && !was) {
      delta.weighted += constraint->weight;
      delta.unweighted++;
    } else if (!now && was) {
      delta.weighted -= constraint->weight;
      delta.unweighted--;
    }
  }
  return delta;
}

static void apply_flip(Problem *problem, int element) {
  const int true_delta = problem->bits[element] ? -1 : 1;
  problem->bits[element] ^= 1;
  for (size_t p = problem->adj_start[element];
       p < problem->adj_start[element + 1]; p++) {
    const int ci = problem->adj[p];
    Constraint *constraint = &problem->constraints[ci];
    constraint->true_count += true_delta;
    if (constraint_is_violated(constraint)) {
      violated_add(problem, ci);
    } else {
      violated_remove(problem, ci);
    }
  }
}

/* Independent from the maintained constraint counters: enumerate the two
 * progression families afresh from n, t, and the current bit vector. */
static int brute_force_violation_count(const Problem *problem) {
  int count = 0;
  for (int diff = 1; 2LL * diff < problem->n; diff++) {
    for (int start = 1; start + 2 * diff <= problem->n; start++) {
      if (problem->bits[start] && problem->bits[start + diff] &&
          problem->bits[start + 2 * diff]) {
        count++;
      }
    }
  }
  const int span = problem->t - 1;
  for (int diff = 1; (long long)span * diff < problem->n; diff++) {
    for (int start = 1; start + span * diff <= problem->n; start++) {
      bool all_zero = true;
      for (int j = 0; j < problem->t; j++) {
        if (problem->bits[start + j * diff]) {
          all_zero = false;
          break;
        }
      }
      count += all_zero;
    }
  }
  return count;
}

static void assert_problem_consistent(const Problem *problem) {
  int listed = 0;
  for (int ci = 0; ci < problem->num_constraints; ci++) {
    const Constraint *constraint = &problem->constraints[ci];
    int true_count = 0;
    int element = constraint->start;
    for (int j = 0; j < constraint->length;
         j++, element += constraint->diff) {
      true_count += problem->bits[element] != 0;
    }
    if (true_count != constraint->true_count) {
      die("self-test true-count mismatch");
    }
    const bool violated = constraint_is_violated_count(constraint, true_count);
    if (violated != (problem->violated_pos[ci] >= 0)) {
      die("self-test violated-position mismatch");
    }
    listed += violated;
  }
  if (listed != problem->num_violated ||
      listed != brute_force_violation_count(problem)) {
    die("self-test brute-force violation mismatch");
  }
  for (int pos = 0; pos < problem->num_violated; pos++) {
    const int ci = problem->violated[pos];
    if (problem->violated_pos[ci] != pos ||
        !constraint_is_violated(&problem->constraints[ci])) {
      die("self-test violated-list mismatch");
    }
  }
}

static void run_self_test(void) {
  Problem problem;
  rng_state = UINT64_C(0x8c3c010cb4754c9d);
  problem_build(&problem, 41, 5);
  for (int element = 1; element <= problem.n; element++) {
    problem.bits[element] = (unsigned char)rng_below(2);
  }
  problem_reset_counts(&problem);
  assert_problem_consistent(&problem);
  for (int step = 0; step < 20000; step++) {
    const int element = 1 + (int)rng_below((unsigned)problem.n);
    const int before = problem.num_violated;
    const Delta delta = delta_for_flip(&problem, element);
    apply_flip(&problem, element);
    if (problem.num_violated != before + delta.unweighted) {
      die("self-test flip delta mismatch");
    }
    if (step % 17 == 0) {
      assert_problem_consistent(&problem);
    }
  }
  problem_destroy(&problem);

  /* Classic valid W(2,3)>8 coloring in the mixed representation. */
  problem_build(&problem, 8, 3);
  const unsigned char valid_bits[8] = {1, 0, 0, 1, 1, 0, 0, 1};
  for (int element = 1; element <= 8; element++) {
    problem.bits[element] = valid_bits[element - 1];
  }
  problem_reset_counts(&problem);
  assert_problem_consistent(&problem);
  if (problem.num_violated != 0) {
    die("self-test rejected known W(2,3)>8 coloring");
  }
  problem_destroy(&problem);
  fprintf(stderr, "SELF-TEST PASSED\n");
}

/* Separate seed parser. Accepts either color order (3,t) or (t,3) and maps
 * the color constrained at length 3 to bit 1. */
static int seed_next_int(FILE *file, long long *out) {
  int ch;
  do {
    ch = fgetc(file);
    if (ch == '#') {
      while (ch != EOF && ch != '\n') {
        ch = fgetc(file);
      }
    }
  } while (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n');
  if (ch == EOF) {
    return 0;
  }
  int sign = 1;
  if (ch == '-') {
    sign = -1;
    ch = fgetc(file);
  }
  if (ch < '0' || ch > '9') {
    die("malformed seed certificate");
  }
  long long value = 0;
  while (ch >= '0' && ch <= '9') {
    if (value > (LLONG_MAX - (ch - '0')) / 10) {
      die("integer overflow in seed certificate");
    }
    value = value * 10 + (ch - '0');
    ch = fgetc(file);
  }
  if (ch == '#') {
    while (ch != EOF && ch != '\n') {
      ch = fgetc(file);
    }
  } else if (ch != EOF && ch != ' ' && ch != '\t' && ch != '\r' &&
             ch != '\n') {
    die("junk in seed certificate");
  }
  *out = sign * value;
  return 1;
}

static long long seed_require_int(FILE *file) {
  long long value;
  if (!seed_next_int(file, &value)) {
    die("short seed certificate");
  }
  return value;
}

static void overlay_seed(Problem *problem, const char *path, bool reverse) {
  FILE *file = fopen(path, "r");
  if (file == NULL) {
    fprintf(stderr, "error: cannot open seed certificate %s\n", path);
    exit(2);
  }
  const long long r = seed_require_int(file);
  if (r != 2) {
    die("seed certificate must have two colors");
  }
  const long long k1 = seed_require_int(file);
  const long long k2 = seed_require_int(file);
  bool color1_is_bit_one;
  if (k1 == 3 && k2 == problem->t) {
    color1_is_bit_one = true;
  } else if (k2 == 3 && k1 == problem->t) {
    color1_is_bit_one = false;
  } else {
    die("seed certificate lengths do not match (3,t)");
  }
  const long long seed_n = seed_require_int(file);
  if (seed_n < 1 || seed_n > problem->n || seed_n > INT_MAX) {
    die("seed certificate length must be in [1,n]");
  }
  unsigned char *seed_bits = xmalloc((size_t)(seed_n + 1));
  for (int element = 1; element <= (int)seed_n; element++) {
    const long long color = seed_require_int(file);
    if (color < 1 || color > 2) {
      die("seed certificate color outside [1,2]");
    }
    seed_bits[element] =
        (unsigned char)(color1_is_bit_one ? color == 1 : color == 2);
  }
  long long extra;
  if (seed_next_int(file, &extra)) {
    die("trailing token in seed certificate");
  }
  fclose(file);
  for (int element = 1; element <= (int)seed_n; element++) {
    const int source = reverse ? (int)seed_n + 1 - element : element;
    problem->bits[element] = seed_bits[source];
  }
  free(seed_bits);
}

static void initialize_digit_sphere(Problem *problem, int base) {
  if (base < 4) {
    base = 8;
  }
  const int digit_limit = base / 2;
  int digits = 1;
  long long power = base;
  while (power < problem->n) {
    if (power > LLONG_MAX / base) {
      break;
    }
    power *= base;
    digits++;
  }
  const int max_sum = digits * (digit_limit - 1) * (digit_limit - 1);
  int *histogram = xcalloc((size_t)(max_sum + 1), sizeof(int));
  int *sums = xmalloc((size_t)(problem->n + 1) * sizeof(int));
  for (int element = 1; element <= problem->n; element++) {
    int x = element - 1;
    int sum = 0;
    bool eligible = true;
    for (int digit = 0; digit < digits; digit++) {
      const int value = x % base;
      x /= base;
      if (value >= digit_limit) {
        eligible = false;
      }
      sum += value * value;
    }
    sums[element] = eligible ? sum : -1;
    if (eligible) {
      histogram[sum]++;
    }
  }
  int best_sum = 0;
  for (int sum = 1; sum <= max_sum; sum++) {
    if (histogram[sum] > histogram[best_sum]) {
      best_sum = sum;
    }
  }
  for (int element = 1; element <= problem->n; element++) {
    problem->bits[element] = (unsigned char)(sums[element] == best_sum);
  }
  fprintf(stderr, "digit template: base=%d sphere=%d size=%d\n", base,
          best_sum, histogram[best_sum]);
  free(sums);
  free(histogram);
}

static void initialize_bits(Problem *problem, StartClass start_class,
                            int density_percent, int parameter,
                            const char *seed_path, bool reverse_seed,
                            int perturb_percent) {
  if (start_class == START_DIGIT) {
    initialize_digit_sphere(problem, parameter);
  } else if (start_class == START_CYCLIC) {
    int modulus = parameter;
    if (modulus < 2) {
      modulus = 2 + (int)rng_below(126);
    }
    unsigned char *pattern = xmalloc((size_t)modulus);
    for (int i = 0; i < modulus; i++) {
      pattern[i] = (unsigned char)((int)rng_below(100) < density_percent);
    }
    for (int element = 1; element <= problem->n; element++) {
      problem->bits[element] = pattern[(element - 1) % modulus];
    }
    fprintf(stderr, "cyclic template: modulus=%d\n", modulus);
    free(pattern);
  } else if (start_class == START_PALINDROME) {
    for (int element = 1; element <= (problem->n + 1) / 2; element++) {
      const unsigned char bit =
          (unsigned char)((int)rng_below(100) < density_percent);
      problem->bits[element] = bit;
      problem->bits[problem->n + 1 - element] = bit;
    }
  } else {
    for (int element = 1; element <= problem->n; element++) {
      problem->bits[element] =
          (unsigned char)((int)rng_below(100) < density_percent);
    }
  }

  if (seed_path != NULL) {
    overlay_seed(problem, seed_path, reverse_seed);
  } else if (start_class == START_SEED) {
    die("seed start class requires -i");
  }

  if (start_class == START_PALINDROME) {
    for (int element = 1; element <= (problem->n + 1) / 2; element++) {
      if ((int)rng_below(100) < perturb_percent) {
        problem->bits[element] ^= 1;
        problem->bits[problem->n + 1 - element] = problem->bits[element];
      }
    }
  } else {
    for (int element = 1; element <= problem->n; element++) {
      if ((int)rng_below(100) < perturb_percent) {
        problem->bits[element] ^= 1;
      }
    }
  }
  problem_reset_counts(problem);
}

static StartClass parse_start_class(const char *name) {
  if (strcmp(name, "random") == 0) {
    return START_RANDOM;
  }
  if (strcmp(name, "seed") == 0) {
    return START_SEED;
  }
  if (strcmp(name, "cyclic") == 0) {
    return START_CYCLIC;
  }
  if (strcmp(name, "palindrome") == 0) {
    return START_PALINDROME;
  }
  if (strcmp(name, "digit") == 0) {
    return START_DIGIT;
  }
  die("unknown start class");
  return START_RANDOM;
}

static const char *start_class_name(StartClass start_class) {
  switch (start_class) {
  case START_RANDOM:
    return "random";
  case START_SEED:
    return "seed";
  case START_CYCLIC:
    return "cyclic";
  case START_PALINDROME:
    return "palindrome";
  case START_DIGIT:
    return "digit";
  }
  return "unknown";
}

static int choose_focus_constraint(const Problem *problem, bool ddfw) {
  int chosen =
      problem->violated[rng_below((unsigned)problem->num_violated)];
  if (ddfw) {
    for (int trial = 0; trial < 2; trial++) {
      const int candidate =
          problem->violated[rng_below((unsigned)problem->num_violated)];
      if (problem->constraints[candidate].weight >
          problem->constraints[chosen].weight) {
        chosen = candidate;
      }
    }
  }
  return chosen;
}

static bool redistribute_weight(Problem *problem, int focus_index) {
  Constraint *focus = &problem->constraints[focus_index];
  const bool random_donor = rng_below(100) == 0;
  int donor = -1;
  int donor_weight = -1;
  unsigned candidates_seen = 0;
  int element = focus->start;
  for (int j = 0; j < focus->length; j++, element += focus->diff) {
    for (size_t p = problem->adj_start[element];
         p < problem->adj_start[element + 1]; p++) {
      const int ci = problem->adj[p];
      Constraint *candidate = &problem->constraints[ci];
      if (ci == focus_index || problem->violated_pos[ci] >= 0 ||
          candidate->weight < 2) {
        continue;
      }
      if (random_donor) {
        candidates_seen++;
        if (rng_below(candidates_seen) == 0) {
          donor = ci;
        }
      } else if (candidate->weight > donor_weight) {
        donor = ci;
        donor_weight = candidate->weight;
      }
    }
  }
  if (donor < 0) {
    return false;
  }
  problem->constraints[donor].weight -= 2;
  focus->weight += 2;
  return true;
}

static double monotonic_seconds(void) {
  struct timespec now;
  if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
    die("clock_gettime failed");
  }
  return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
}

static int write_candidate(const Problem *problem, const char *path,
                           const char *mode, StartClass start_class,
                           uint64_t seed, long long flips,
                           long long weight_updates, const char *seed_path,
                           int perturb_percent, bool reverse_seed) {
  FILE *file = fopen(path, "w");
  if (file == NULL) {
    fprintf(stderr, "error: cannot write candidate %s\n", path);
    return 2;
  }
  fprintf(file,
          "# UNVERIFIED t2_sls candidate: n=%d t=%d mode=%s class=%s "
          "rngseed=%llu flips=%lld weight_updates=%lld seed=%s P=%d R=%d\n",
          problem->n, problem->t, mode, start_class_name(start_class),
          (unsigned long long)seed, flips, weight_updates,
          seed_path != NULL ? seed_path : "-", perturb_percent,
          reverse_seed ? 1 : 0);
  fprintf(file, "2\n3 %d\n%d\n", problem->t, problem->n);
  for (int element = 1; element <= problem->n; element++) {
    const int color = problem->bits[element] ? 1 : 2;
    fprintf(file, "%d%s", color,
            element % 60 == 0 || element == problem->n ? "\n" : " ");
  }
  if (fclose(file) != 0) {
    fprintf(stderr, "error: failed closing candidate %s\n", path);
    return 2;
  }
  return 0;
}

static void print_usage(const char *program) {
  fprintf(stderr,
          "usage: %s -n N -t T -m walksat|ddfw -f MAXFLIPS -s SEED "
          "-o CANDIDATE\n"
          "       [-c random|seed|cyclic|palindrome|digit] [-i seed-cert]\n"
          "       [-P perturb-percent] [-R] [-q template-parameter]\n"
          "       [-D color1-density-percent] [-l seconds] "
          "[-p progress-flips]\n"
          "       %s --self-test\n",
          program, program);
}

int main(int argc, char **argv) {
  if (argc == 2 && strcmp(argv[1], "--self-test") == 0) {
    run_self_test();
    return 0;
  }

  int n = 0;
  int t = 0;
  long long max_flips = 200000000LL;
  uint64_t seed = 12345;
  const char *mode = "ddfw";
  const char *start_name = "random";
  const char *seed_path = NULL;
  const char *output_path = NULL;
  int perturb_percent = 0;
  int density_percent = 20;
  int template_parameter = 0;
  bool reverse_seed = false;
  double time_limit = 0.0;
  long long progress_interval = 1000000;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
      n = (int)parse_ll(argv[++i], "n");
    } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
      t = (int)parse_ll(argv[++i], "t");
    } else if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) {
      mode = argv[++i];
    } else if (strcmp(argv[i], "-f") == 0 && i + 1 < argc) {
      max_flips = parse_ll(argv[++i], "max flips");
    } else if (strcmp(argv[i], "-s") == 0 && i + 1 < argc) {
      seed = parse_u64(argv[++i], "seed");
    } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
      start_name = argv[++i];
    } else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
      seed_path = argv[++i];
    } else if (strcmp(argv[i], "-P") == 0 && i + 1 < argc) {
      perturb_percent = (int)parse_ll(argv[++i], "perturb percent");
    } else if (strcmp(argv[i], "-R") == 0) {
      reverse_seed = true;
    } else if (strcmp(argv[i], "-q") == 0 && i + 1 < argc) {
      template_parameter = (int)parse_ll(argv[++i], "template parameter");
    } else if (strcmp(argv[i], "-D") == 0 && i + 1 < argc) {
      density_percent = (int)parse_ll(argv[++i], "density percent");
    } else if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
      time_limit = (double)parse_ll(argv[++i], "time limit");
    } else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
      progress_interval = parse_ll(argv[++i], "progress interval");
    } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
      output_path = argv[++i];
    } else {
      print_usage(argv[0]);
      return 2;
    }
  }

  if (n < 3 || t < 3 || t > n || max_flips < 1 || output_path == NULL ||
      perturb_percent < 0 || perturb_percent > 100 || density_percent < 0 ||
      density_percent > 100 || time_limit < 0.0 || progress_interval < 0 ||
      (strcmp(mode, "walksat") != 0 && strcmp(mode, "ddfw") != 0)) {
    print_usage(argv[0]);
    return 2;
  }
  const bool ddfw = strcmp(mode, "ddfw") == 0;
  const StartClass start_class = parse_start_class(start_name);
  rng_state = seed ^ UINT64_C(0x9e3779b97f4a7c15);
  if (rng_state == 0) {
    rng_state = UINT64_C(0xd1b54a32d192ed03);
  }

  Problem problem;
  problem_build(&problem, n, t);
  initialize_bits(&problem, start_class, density_percent, template_parameter,
                  seed_path, reverse_seed, perturb_percent);
  int best_violations = problem.num_violated;
  long long last_improvement = 0;
  long long flips = 0;
  long long weight_updates = 0;
  long long search_steps = 0;
  double noise = 0.25;
  const double started = monotonic_seconds();
  bool timed_out = false;

  fprintf(stderr,
          "start: n=%d t=%d constraints=%d violations=%d mode=%s class=%s "
          "seed=%llu\n",
          n, t, problem.num_constraints, problem.num_violated, mode,
          start_class_name(start_class), (unsigned long long)seed);

  while (problem.num_violated > 0 && flips < max_flips) {
    search_steps++;
    if ((search_steps & 65535) == 0 && time_limit > 0.0 &&
        monotonic_seconds() - started >= time_limit) {
      timed_out = true;
      break;
    }
    const int focus_index = choose_focus_constraint(&problem, ddfw);
    const Constraint *focus = &problem.constraints[focus_index];

    int best_element = -1;
    Delta best_delta = {0, 0};
    unsigned ties = 0;
    int element = focus->start;
    for (int j = 0; j < focus->length; j++, element += focus->diff) {
      const Delta delta = delta_for_flip(&problem, element);
      const bool aspiration =
          problem.num_violated + delta.unweighted < best_violations;
      if (problem.tabu_until[element] > flips && !aspiration) {
        continue;
      }
      if (best_element < 0 || delta.weighted < best_delta.weighted) {
        best_element = element;
        best_delta = delta;
        ties = 1;
      } else if (delta.weighted == best_delta.weighted &&
                 rng_below(++ties) == 0) {
        best_element = element;
        best_delta = delta;
      }
    }

    if (best_element >= 0 && ddfw && best_delta.weighted >= 0 &&
        rng_unit() >= noise && redistribute_weight(&problem, focus_index)) {
      weight_updates++;
      continue;
    }

    if (best_element < 0 || rng_unit() < noise) {
      const int offset = (int)rng_below((unsigned)focus->length);
      best_element = focus->start + offset * focus->diff;
      best_delta = delta_for_flip(&problem, best_element);
    }
    (void)best_delta;
    apply_flip(&problem, best_element);
    flips++;
    problem.tabu_until[best_element] = flips + TABU_TENURE;

    if (problem.num_violated < best_violations) {
      best_violations = problem.num_violated;
      last_improvement = flips;
      noise = 0.25 + (noise - 0.25) * 0.5;
      fprintf(stderr,
              "improve: flips=%lld best=%d current=%d noise=%.4f "
              "weight_updates=%lld\n",
              flips, best_violations, problem.num_violated, noise,
              weight_updates);
    } else if (flips - last_improvement >= 1000000) {
      noise *= 1.2;
      if (noise > 0.60) {
        noise = 0.60;
      }
      last_improvement = flips;
    }
    if (progress_interval > 0 && flips % progress_interval == 0) {
      fprintf(stderr,
              "progress: flips=%lld best=%d current=%d noise=%.4f "
              "weight_updates=%lld seconds=%.3f\n",
              flips, best_violations, problem.num_violated, noise,
              weight_updates, monotonic_seconds() - started);
    }
  }

  const double elapsed = monotonic_seconds() - started;
  if (problem.num_violated == 0) {
    const int status =
        write_candidate(&problem, output_path, mode, start_class, seed, flips,
                        weight_updates, seed_path, perturb_percent,
                        reverse_seed);
    fprintf(stderr,
            "CANDIDATE n=%d t=%d flips=%lld weight_updates=%lld "
            "seconds=%.3f path=%s (MUST RUN verifier.c)\n",
            n, t, flips, weight_updates, elapsed, output_path);
    problem_destroy(&problem);
    return status;
  }

  fprintf(stderr,
          "exhausted: n=%d t=%d flips=%lld best=%d final=%d "
          "weight_updates=%lld seconds=%.3f%s\n",
          n, t, flips, best_violations, problem.num_violated, weight_updates,
          elapsed, timed_out ? " timed_out" : "");
  problem_destroy(&problem);
  return 1;
}
