/* t2_cadical.c — CaDiCaL API lane for mixed w(2;3,t).
 *
 * Generator-side plain C11 using CaDiCaL's C API. Shares no code with the
 * frozen verifier or M2/T4 sources. A SAT model is emitted as an explicitly
 * UNVERIFIED complete certificate and must be checked by verifier.c.
 *
 * x_i = 1 means verifier color 1 (forbidden 3-AP);
 * x_i = 0 means verifier color 2 (forbidden t-AP).
 *
 * Build (Homebrew CaDiCaL):
 *   cc -std=c11 -O3 -I/opt/homebrew/include -c t2_cadical.c
 *   c++ t2_cadical.o /opt/homebrew/lib/libcadical.a -o t2_cadical
 *
 * Usage:
 *   t2_cadical -n N -t T -o CANDIDATE [-i seed-cert] [-l seconds]
 *               [--no-reflection | --palindrome] [--seed-right]
 *               [--hamming K] [--stats]
 *
 * Exit 0: SAT candidate written.
 * Exit 1: UNSAT.
 * Exit 3: UNKNOWN/terminated.
 * Exit 2: malformed input or I/O failure.
 */

#include <ccadical.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct TerminationState {
  double deadline;
  uint64_t calls;
} TerminationState;

typedef struct EncodingStats {
  int variables;
  int auxiliaries;
  int64_t clauses;
  int64_t literals;
} EncodingStats;

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

static long long parse_ll(const char *text, const char *what) {
  char *end = NULL;
  errno = 0;
  const long long value = strtoll(text, &end, 10);
  if (errno != 0 || end == text || *end != '\0') {
    fprintf(stderr, "error: invalid %s: %s\n", what, text);
    exit(2);
  }
  return value;
}

static double monotonic_seconds(void) {
  struct timespec now;
  if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
    die("clock_gettime failed");
  }
  return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
}

static int terminate_callback(void *opaque) {
  TerminationState *state = opaque;
  state->calls++;
  return state->deadline > 0.0 && monotonic_seconds() >= state->deadline;
}

static void add_clause(CCaDiCaL *solver, EncodingStats *stats,
                       const int *literals, int count) {
  for (int i = 0; i < count; i++) {
    ccadical_add(solver, literals[i]);
  }
  ccadical_add(solver, 0);
  stats->clauses++;
  stats->literals += count;
}

static void add_3ap_clauses(CCaDiCaL *solver, EncodingStats *stats, int n) {
  for (int diff = 1; 2LL * diff < n; diff++) {
    for (int start = 1; start + 2 * diff <= n; start++) {
      const int clause[3] = {-start, -(start + diff), -(start + 2 * diff)};
      add_clause(solver, stats, clause, 3);
    }
  }
}

static void add_tap_clauses(CCaDiCaL *solver, EncodingStats *stats, int n,
                            int t) {
  int *clause = xmalloc((size_t)t * sizeof(*clause));
  const int span = t - 1;
  for (int diff = 1; (long long)span * diff < n; diff++) {
    for (int start = 1; start + span * diff <= n; start++) {
      for (int j = 0; j < t; j++) {
        clause[j] = start + j * diff;
      }
      add_clause(solver, stats, clause, t);
    }
  }
  free(clause);
}

/* Add x_1...x_n <=lex x_n...x_1.
 *
 * Prefix variable p_i means all reflected pairs through i are equal. While
 * p_{i-1} is true, (x_i,x_j)=(1,0) is forbidden. The last prefix variable is
 * unnecessary because no later pair consumes it.
 */
static void add_reflection_lex_leader(CCaDiCaL *solver, EncodingStats *stats,
                                      int n) {
  const int pairs = n / 2;
  int previous_prefix = 0; /* conceptual true for the first pair */
  for (int pair = 1; pair <= pairs; pair++) {
    const int left = pair;
    const int right = n + 1 - pair;
    if (previous_prefix == 0) {
      const int guard[2] = {-left, right};
      add_clause(solver, stats, guard, 2);
    } else {
      const int guard[3] = {-previous_prefix, -left, right};
      add_clause(solver, stats, guard, 3);
    }
    if (pair == pairs) {
      break;
    }

    const int prefix = n + pair;
    stats->auxiliaries++;
    if (previous_prefix != 0) {
      const int implies_previous[2] = {-prefix, previous_prefix};
      add_clause(solver, stats, implies_previous, 2);
    }
    const int implies_equal_ab[3] = {-prefix, -left, right};
    const int implies_equal_ba[3] = {-prefix, left, -right};
    add_clause(solver, stats, implies_equal_ab, 3);
    add_clause(solver, stats, implies_equal_ba, 3);

    if (previous_prefix == 0) {
      const int equal_one[3] = {-left, -right, prefix};
      const int equal_zero[3] = {left, right, prefix};
      add_clause(solver, stats, equal_one, 3);
      add_clause(solver, stats, equal_zero, 3);
    } else {
      const int equal_one[4] = {-previous_prefix, -left, -right, prefix};
      const int equal_zero[4] = {-previous_prefix, left, right, prefix};
      add_clause(solver, stats, equal_one, 4);
      add_clause(solver, stats, equal_zero, 4);
    }
    previous_prefix = prefix;
  }
  stats->variables = n + stats->auxiliaries;
}

static void add_palindrome(CCaDiCaL *solver, EncodingStats *stats, int n) {
  for (int left = 1; left <= n / 2; left++) {
    const int right = n + 1 - left;
    const int forward[2] = {-left, right};
    const int backward[2] = {left, -right};
    add_clause(solver, stats, forward, 2);
    add_clause(solver, stats, backward, 2);
  }
}

/* Sinz sequential-counter encoding of at most k true mismatch literals.
 * A positive phase means x_i=true in the seed; therefore the corresponding
 * mismatch literal is -x_i. A negative phase makes +x_i the mismatch literal.
 * This is a search streamliner only: UNSAT under this bound is not a result.
 */
static void add_hamming_at_most(CCaDiCaL *solver, EncodingStats *stats,
                                const signed char *phases, int n, int k) {
  if (k >= n) {
    return;
  }
  if (k == 0) {
    for (int variable = 1; variable <= n; variable++) {
      const int mismatch = phases[variable] > 0 ? -variable : variable;
      const int unit[1] = {-mismatch};
      add_clause(solver, stats, unit, 1);
    }
    return;
  }

  const long long counter_count = (long long)(n - 1) * k;
  if (counter_count > INT_MAX - stats->variables) {
    die("Hamming counter variable count overflow");
  }
  const int first_counter = stats->variables + 1;
  stats->variables += (int)counter_count;
  stats->auxiliaries += (int)counter_count;

#define COUNTER(i, j) (first_counter + ((i)-1) * k + ((j)-1))
#define MISMATCH(i) (phases[(i)] > 0 ? -(i) : (i))

  for (int i = 1; i <= n - 1; i++) {
    const int introduce[2] = {-MISMATCH(i), COUNTER(i, 1)};
    add_clause(solver, stats, introduce, 2);
  }
  for (int i = 2; i <= n - 1; i++) {
    const int propagate[2] = {-COUNTER(i - 1, 1), COUNTER(i, 1)};
    add_clause(solver, stats, propagate, 2);
  }
  for (int j = 2; j <= k; j++) {
    for (int i = 2; i <= n - 1; i++) {
      const int advance[3] = {
          -MISMATCH(i),
          -COUNTER(i - 1, j - 1),
          COUNTER(i, j),
      };
      const int propagate[2] = {
          -COUNTER(i - 1, j),
          COUNTER(i, j),
      };
      add_clause(solver, stats, advance, 3);
      add_clause(solver, stats, propagate, 2);
    }
  }
  for (int i = 2; i <= n; i++) {
    const int overflow[2] = {-MISMATCH(i), -COUNTER(i - 1, k)};
    add_clause(solver, stats, overflow, 2);
  }

#undef MISMATCH
#undef COUNTER
}

static int seed_next_int(FILE *file, long long *out) {
  int ch;
  for (;;) {
    ch = fgetc(file);
    if (ch == EOF) {
      return 0;
    }
    if (ch == '#') {
      while (ch != EOF && ch != '\n') {
        ch = fgetc(file);
      }
      continue;
    }
    if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
      continue;
    }
    break;
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

/* Returns phases in 1..n: +1 means x=true, -1 means x=false. */
static signed char *read_seed_phases(const char *path, int n, int t,
                                     bool seed_right, int *seed_length) {
  signed char *phases = xmalloc((size_t)(n + 1) * sizeof(*phases));
  for (int variable = 1; variable <= n; variable++) {
    phases[variable] = -1; /* deterministic extension: color 2 */
  }
  *seed_length = 0;
  if (path == NULL) {
    return phases;
  }

  FILE *file = fopen(path, "r");
  if (file == NULL) {
    die("cannot open seed certificate");
  }
  if (seed_require_int(file) != 2) {
    die("seed certificate must have two colors");
  }
  const long long k1 = seed_require_int(file);
  const long long k2 = seed_require_int(file);
  bool color1_is_true;
  if (k1 == 3 && k2 == t) {
    color1_is_true = true;
  } else if (k2 == 3 && k1 == t) {
    color1_is_true = false;
  } else {
    die("seed certificate lengths do not match (3,t)");
  }
  const long long source_n = seed_require_int(file);
  if (source_n < 1 || source_n > n || source_n > INT_MAX) {
    die("seed certificate length must be in [1,n]");
  }
  *seed_length = (int)source_n;
  const int offset = seed_right ? n - *seed_length : 0;
  for (int variable = 1; variable <= *seed_length; variable++) {
    const long long color = seed_require_int(file);
    if (color < 1 || color > 2) {
      die("seed certificate color outside [1,2]");
    }
    const bool is_true = color1_is_true ? color == 1 : color == 2;
    phases[offset + variable] = (signed char)(is_true ? 1 : -1);
  }
  long long extra;
  if (seed_next_int(file, &extra)) {
    die("trailing token in seed certificate");
  }
  fclose(file);
  return phases;
}

static int write_candidate(CCaDiCaL *solver, const char *path, int n, int t,
                           bool reflection, bool palindrome,
                           const char *seed_path, int seed_length,
                           bool seed_right, int hamming_limit,
                           const EncodingStats *stats) {
  FILE *file = fopen(path, "w");
  if (file == NULL) {
    fprintf(stderr, "error: cannot write candidate %s\n", path);
    return 2;
  }
  fprintf(file,
          "# UNVERIFIED t2_cadical candidate: n=%d t=%d reflection=%d "
          "palindrome=%d seed=%s seed_length=%d seed_right=%d hamming=%d "
          "variables=%d clauses=%lld\n",
          n, t, reflection ? 1 : 0, palindrome ? 1 : 0,
          seed_path != NULL ? seed_path : "-", seed_length,
          seed_right ? 1 : 0, hamming_limit, stats->variables,
          (long long)stats->clauses);
  fprintf(file, "2\n3 %d\n%d\n", t, n);
  for (int variable = 1; variable <= n; variable++) {
    const int color = ccadical_val(solver, variable) > 0 ? 1 : 2;
    fprintf(file, "%d%s", color,
            variable % 60 == 0 || variable == n ? "\n" : " ");
  }
  if (fclose(file) != 0) {
    fprintf(stderr, "error: failed closing candidate %s\n", path);
    return 2;
  }
  return 0;
}

static void print_usage(const char *program) {
  fprintf(stderr,
          "usage: %s -n N -t T -o CANDIDATE [-i seed-cert] [-l seconds]\n"
          "       [--no-reflection | --palindrome] [--seed-right]\n"
          "       [--hamming K] [--stats]\n",
          program);
}

int main(int argc, char **argv) {
  int n = 0;
  int t = 0;
  const char *output_path = NULL;
  const char *seed_path = NULL;
  long long time_limit = 0;
  bool reflection = true;
  bool palindrome = false;
  bool seed_right = false;
  bool print_stats = false;
  long long hamming_limit = -1;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
      n = (int)parse_ll(argv[++i], "n");
    } else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
      t = (int)parse_ll(argv[++i], "t");
    } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
      output_path = argv[++i];
    } else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
      seed_path = argv[++i];
    } else if (strcmp(argv[i], "-l") == 0 && i + 1 < argc) {
      time_limit = parse_ll(argv[++i], "time limit");
    } else if (strcmp(argv[i], "--no-reflection") == 0) {
      reflection = false;
    } else if (strcmp(argv[i], "--palindrome") == 0) {
      palindrome = true;
      reflection = false;
    } else if (strcmp(argv[i], "--seed-right") == 0) {
      seed_right = true;
    } else if (strcmp(argv[i], "--hamming") == 0 && i + 1 < argc) {
      hamming_limit = parse_ll(argv[++i], "Hamming limit");
    } else if (strcmp(argv[i], "--stats") == 0) {
      print_stats = true;
    } else {
      print_usage(argv[0]);
      return 2;
    }
  }
  if (n < 3 || t < 3 || t > n || output_path == NULL || time_limit < 0 ||
      hamming_limit > INT_MAX || hamming_limit < -1 ||
      ((seed_right || hamming_limit >= 0) && seed_path == NULL)) {
    print_usage(argv[0]);
    return 2;
  }

  CCaDiCaL *solver = ccadical_init();
  if (solver == NULL) {
    die("ccadical_init failed");
  }
  EncodingStats stats = {
      .variables = n,
      .auxiliaries = 0,
      .clauses = 0,
      .literals = 0,
  };
  add_3ap_clauses(solver, &stats, n);
  add_tap_clauses(solver, &stats, n, t);
  if (reflection) {
    add_reflection_lex_leader(solver, &stats, n);
  } else if (palindrome) {
    add_palindrome(solver, &stats, n);
  }

  int seed_length = 0;
  signed char *phases =
      read_seed_phases(seed_path, n, t, seed_right, &seed_length);
  for (int variable = 1; variable <= n; variable++) {
    ccadical_phase(solver, phases[variable] > 0 ? variable : -variable);
  }
  if (hamming_limit >= 0) {
    add_hamming_at_most(solver, &stats, phases, n, (int)hamming_limit);
  }
  free(phases);

  TerminationState termination = {
      .deadline =
          time_limit > 0 ? monotonic_seconds() + (double)time_limit : 0.0,
      .calls = 0,
  };
  if (time_limit > 0) {
    ccadical_set_terminate(solver, &termination, terminate_callback);
  }

  fprintf(stderr,
          "solve: n=%d t=%d variables=%d auxiliaries=%d clauses=%lld "
          "literals=%lld reflection=%d palindrome=%d seed_length=%d "
          "seed_right=%d hamming=%lld cadical=%s\n",
          n, t, stats.variables, stats.auxiliaries, (long long)stats.clauses,
          (long long)stats.literals, reflection ? 1 : 0, palindrome ? 1 : 0,
          seed_length, seed_right ? 1 : 0, hamming_limit,
          ccadical_signature());
  const double started = monotonic_seconds();
  const int result = ccadical_solve(solver);
  const double elapsed = monotonic_seconds() - started;
  if (print_stats) {
    ccadical_print_statistics(solver);
  }

  int exit_code;
  if (result == 10) {
    exit_code = write_candidate(solver, output_path, n, t, reflection,
                                palindrome, seed_path, seed_length, seed_right,
                                (int)hamming_limit, &stats);
    fprintf(stderr,
            "SAT n=%d t=%d seconds=%.6f path=%s (MUST RUN verifier.c)\n", n,
            t, elapsed, output_path);
  } else if (result == 20) {
    fprintf(stderr, "UNSAT n=%d t=%d seconds=%.6f\n", n, t, elapsed);
    exit_code = 1;
  } else {
    fprintf(stderr,
            "UNKNOWN n=%d t=%d seconds=%.6f terminate_callbacks=%llu\n", n, t,
            elapsed, (unsigned long long)termination.calls);
    exit_code = 3;
  }
  ccadical_release(solver);
  return exit_code;
}
