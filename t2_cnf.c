/* t2_cnf.c — phase-seeded DIMACS emitter for mixed w(2;3,t).
 *
 * Generator-side C99.  This shares no code with verifier.c and emits only a
 * decision instance.  Any SAT model must be decoded and accepted by the
 * frozen standalone verifier before it is a result.
 *
 * Logical variable x_i is true for verifier color 1 (which must avoid
 * 3-APs) and false for color 2 (which must avoid t-APs).  With a seed, each
 * primary variable is polarity-flipped when necessary so that Kissat's
 * all-false default phase is exactly the supplied coloring.  This changes
 * only the encoding, not the represented formula.
 *
 * With no seed, a solution-preserving reflection lex leader
 * x_1...x_n <=lex x_n...x_1 is emitted.  A fixed phase seed already chooses
 * an orientation, so seeded instances omit the lex leader rather than make
 * it fight the preferred phase.
 *
 * Usage:
 *   t2_cnf N T [seed-certificate] > problem.cnf
 */

#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int n;
static int t;
static unsigned char *seed_color1;

static void die(const char *message) {
  fprintf(stderr, "error: %s\n", message);
  exit(2);
}

static void *xcalloc(size_t count, size_t bytes) {
  if (bytes != 0 && count > SIZE_MAX / bytes) {
    die("allocation size overflow");
  }
  void *result = calloc(count, bytes);
  if (result == NULL && count != 0 && bytes != 0) {
    die("out of memory");
  }
  return result;
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

/* Read the next integer, skipping # comments. */
static int next_int(FILE *file, long long *out) {
  int ch;
  do {
    ch = fgetc(file);
    if (ch == '#') {
      while ((ch = fgetc(file)) != EOF && ch != '\n') {
      }
    }
  } while (ch != EOF && (ch == ' ' || ch == '\t' || ch == '\r' ||
                         ch == '\n'));
  if (ch == EOF) {
    return 0;
  }
  if (ungetc(ch, file) == EOF) {
    die("ungetc failed");
  }

  char token[128];
  size_t length = 0;
  while ((ch = fgetc(file)) != EOF && ch != ' ' && ch != '\t' &&
         ch != '\r' && ch != '\n' && ch != '#') {
    if (length + 1 >= sizeof(token)) {
      die("overlong integer in seed");
    }
    token[length++] = (char)ch;
  }
  if (ch == '#') {
    while ((ch = fgetc(file)) != EOF && ch != '\n') {
    }
  }
  token[length] = '\0';
  if (length == 0) {
    die("malformed seed certificate");
  }
  char *end = NULL;
  errno = 0;
  const long long value = strtoll(token, &end, 10);
  if (errno != 0 || end == token || *end != '\0') {
    die("malformed integer in seed certificate");
  }
  *out = value;
  return 1;
}

static long long require_int(FILE *file) {
  long long value = 0;
  if (!next_int(file, &value)) {
    die("short seed certificate");
  }
  return value;
}

static void read_seed(const char *path) {
  FILE *file = fopen(path, "r");
  if (file == NULL) {
    die("cannot open seed certificate");
  }
  if (require_int(file) != 2 || require_int(file) != 3 ||
      require_int(file) != t) {
    die("seed header does not match r=2, lengths=(3,t)");
  }
  const long long seed_n = require_int(file);
  if (seed_n < 1 || seed_n > n) {
    die("seed length must be in [1,N]");
  }
  seed_color1 = xcalloc((size_t)n + 1, sizeof(*seed_color1));
  for (int i = 1; i <= n; i++) {
    seed_color1[i] = 0; /* deterministic extension is color 2 */
  }
  for (long long i = 1; i <= seed_n; i++) {
    const long long color = require_int(file);
    if (color != 1 && color != 2) {
      die("seed color outside [1,2]");
    }
    seed_color1[i] = (unsigned char)(color == 1);
  }
  long long extra = 0;
  if (next_int(file, &extra)) {
    die("trailing integer in seed certificate");
  }
  if (fclose(file) != 0) {
    die("failed closing seed certificate");
  }
}

/* Emit a logical literal over a primary variable through the seed-dependent
 * polarity map.  Auxiliaries are emitted directly by emit_aux(). */
static void emit_primary(int variable, bool positive) {
  const bool flip = seed_color1 != NULL && seed_color1[variable] != 0;
  const bool encoded_positive = positive != flip;
  printf("%d ", encoded_positive ? variable : -variable);
}

static void emit_aux(int literal) { printf("%d ", literal); }

static int64_t count_3aps(void) {
  int64_t count = 0;
  for (int diff = 1; 2LL * diff < n; diff++) {
    count += n - 2LL * diff;
  }
  return count;
}

static int64_t count_taps(void) {
  int64_t count = 0;
  const int span = t - 1;
  for (int diff = 1; (int64_t)span * diff < n; diff++) {
    count += n - (int64_t)span * diff;
  }
  return count;
}

static int64_t count_reflection_clauses(void) {
  const int pairs = n / 2;
  int64_t clauses = 0;
  for (int pair = 1; pair <= pairs; pair++) {
    clauses++; /* guarded order clause */
    if (pair == pairs) {
      break;
    }
    if (pair > 1) {
      clauses++; /* prefix implies previous prefix */
    }
    clauses += 4; /* prefix implies equality, equality implies prefix */
  }
  return clauses;
}

static void emit_progression_clauses(void) {
  for (int diff = 1; 2LL * diff < n; diff++) {
    for (int start = 1; start + 2 * diff <= n; start++) {
      emit_primary(start, false);
      emit_primary(start + diff, false);
      emit_primary(start + 2 * diff, false);
      puts("0");
    }
  }
  const int span = t - 1;
  for (int diff = 1; (int64_t)span * diff < n; diff++) {
    for (int start = 1; start + span * diff <= n; start++) {
      for (int j = 0; j < t; j++) {
        emit_primary(start + j * diff, true);
      }
      puts("0");
    }
  }
}

static void emit_reflection_lex_leader(void) {
  const int pairs = n / 2;
  int previous_prefix = 0;
  for (int pair = 1; pair <= pairs; pair++) {
    const int left = pair;
    const int right = n + 1 - pair;
    if (previous_prefix != 0) {
      emit_aux(-previous_prefix);
    }
    emit_primary(left, false);
    emit_primary(right, true);
    puts("0");
    if (pair == pairs) {
      break;
    }

    const int prefix = n + pair;
    if (previous_prefix != 0) {
      emit_aux(-prefix);
      emit_aux(previous_prefix);
      puts("0");
    }
    emit_aux(-prefix);
    emit_primary(left, false);
    emit_primary(right, true);
    puts("0");
    emit_aux(-prefix);
    emit_primary(left, true);
    emit_primary(right, false);
    puts("0");

    if (previous_prefix != 0) {
      emit_aux(-previous_prefix);
    }
    emit_primary(left, false);
    emit_primary(right, false);
    emit_aux(prefix);
    puts("0");
    if (previous_prefix != 0) {
      emit_aux(-previous_prefix);
    }
    emit_primary(left, true);
    emit_primary(right, true);
    emit_aux(prefix);
    puts("0");
    previous_prefix = prefix;
  }
}

int main(int argc, char **argv) {
  if (argc != 3 && argc != 4) {
    fprintf(stderr, "usage: %s N T [seed-certificate]\n", argv[0]);
    return 2;
  }
  const long long parsed_n = parse_ll(argv[1], "N");
  const long long parsed_t = parse_ll(argv[2], "T");
  if (parsed_n < 3 || parsed_n > INT_MAX || parsed_t < 3 ||
      parsed_t > parsed_n || parsed_t > INT_MAX) {
    die("need N >= T >= 3");
  }
  n = (int)parsed_n;
  t = (int)parsed_t;
  if (argc == 4) {
    read_seed(argv[3]);
  }

  const int pairs = n / 2;
  const int variables =
      seed_color1 != NULL ? n : n + (pairs > 0 ? pairs - 1 : 0);
  const int64_t progression_clauses = count_3aps() + count_taps();
  const int64_t reflection_clauses =
      seed_color1 == NULL ? count_reflection_clauses() : 0;
  printf("p cnf %d %lld\n", variables,
         (long long)(progression_clauses + reflection_clauses));
  emit_progression_clauses();
  if (seed_color1 == NULL) {
    emit_reflection_lex_leader();
  }

  fprintf(stderr,
          "emitted: n=%d t=%d variables=%d clauses=%lld phase_seeded=%d\n",
          n, t, variables,
          (long long)(progression_clauses + reflection_clauses),
          seed_color1 != NULL ? 1 : 0);
  free(seed_color1);
  return 0;
}
