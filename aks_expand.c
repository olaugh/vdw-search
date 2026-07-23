/* aks_expand.c — expand AKS appendix certificate notation.
 *
 * Independent generator-side utility; shares no code with verifier.c.
 *
 * The input is a stream of 0, 1, 0^{N}, and 1^{N}, with whitespace and
 * #-comments ignored. In Ahmed–Kullmann–Snevily's notation, 0 is the block
 * avoiding 3-APs and 1 is the block avoiding t-APs. Output uses this
 * repository's verifier format: certificate color 1 has k=3 and color 2 has
 * k=t.
 *
 * Usage:
 *   aks_expand T EXPECTED_LENGTH compact.txt certificate.txt
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static void fail(const char *message) {
  fprintf(stderr, "error: %s\n", message);
  exit(2);
}

static long parse_long(const char *text, const char *what) {
  char *end = NULL;
  errno = 0;
  long value = strtol(text, &end, 10);
  if (errno != 0 || end == text || *end != '\0') {
    fprintf(stderr, "error: invalid %s: %s\n", what, text);
    exit(2);
  }
  return value;
}

static int next_nonspace(FILE *file) {
  for (;;) {
    const int ch = fgetc(file);
    if (ch == EOF) {
      return EOF;
    }
    if (ch == '#') {
      int comment_ch = ch;
      while (comment_ch != EOF && comment_ch != '\n') {
        comment_ch = fgetc(file);
      }
      continue;
    }
    if (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n') {
      continue;
    }
    return ch;
  }
}

static long read_exponent(FILE *file) {
  if (next_nonspace(file) != '{') {
    fail("expected '{' after '^'");
  }
  long exponent = 0;
  int digits = 0;
  for (;;) {
    const int ch = fgetc(file);
    if (ch == '}') {
      break;
    }
    if (ch < '0' || ch > '9') {
      fail("non-digit in exponent");
    }
    if (exponent > (LONG_MAX - (ch - '0')) / 10) {
      fail("exponent overflow");
    }
    exponent = exponent * 10 + (ch - '0');
    digits++;
  }
  if (digits == 0 || exponent < 1) {
    fail("exponent must be positive");
  }
  return exponent;
}

int main(int argc, char **argv) {
  if (argc != 5) {
    fprintf(stderr,
            "usage: %s T EXPECTED_LENGTH compact.txt certificate.txt\n",
            argv[0]);
    return 2;
  }
  const long t = parse_long(argv[1], "t");
  const long expected_length = parse_long(argv[2], "expected length");
  if (t < 3 || expected_length < 1 || expected_length > INT_MAX) {
    fail("t or expected length outside supported range");
  }

  FILE *input = fopen(argv[3], "r");
  if (input == NULL) {
    fail("cannot open compact input");
  }
  size_t capacity = 1024;
  size_t length = 0;
  unsigned char *colors = malloc(capacity);
  if (colors == NULL) {
    fail("out of memory");
  }

  for (;;) {
    const int bit = next_nonspace(input);
    if (bit == EOF) {
      break;
    }
    if (bit != '0' && bit != '1') {
      fail("compact input contains a token other than 0 or 1");
    }
    long repeat = 1;
    const int next = fgetc(input);
    if (next == '^') {
      repeat = read_exponent(input);
    } else if (next != EOF) {
      if (ungetc(next, input) == EOF) {
        fail("ungetc failed");
      }
    }
    if ((unsigned long)repeat > SIZE_MAX - length) {
      fail("expanded length overflow");
    }
    const size_t needed = length + (size_t)repeat;
    if (needed > capacity) {
      while (capacity < needed) {
        if (capacity > SIZE_MAX / 2) {
          fail("expanded allocation overflow");
        }
        capacity *= 2;
      }
      unsigned char *larger = realloc(colors, capacity);
      if (larger == NULL) {
        fail("out of memory");
      }
      colors = larger;
    }
    /* AKS 0=P_0 avoids 3-APs; AKS 1=P_1 avoids t-APs. */
    const unsigned char verifier_color = bit == '0' ? 1 : 2;
    for (long i = 0; i < repeat; i++) {
      colors[length++] = verifier_color;
    }
  }
  fclose(input);

  if (length != (size_t)expected_length) {
    fprintf(stderr, "error: expanded length %zu, expected %ld\n", length,
            expected_length);
    free(colors);
    return 2;
  }

  FILE *output = fopen(argv[4], "w");
  if (output == NULL) {
    free(colors);
    fail("cannot open certificate output");
  }
  fprintf(output,
          "# Transcribed from Ahmed-Kullmann-Snevily, arXiv:1102.5433v4,\n"
          "# Appendix: Further lower bounds for w(2;3,t).\n"
          "# t=%ld certificate_length=%zu claimed_bound: w(2;3,%ld) > %ld\n",
          t, length, t, expected_length);
  fprintf(output, "2\n3 %ld\n%zu\n", t, length);
  for (size_t i = 0; i < length; i++) {
    fprintf(output, "%u%s", (unsigned)colors[i],
            (i + 1) % 60 == 0 || i + 1 == length ? "\n" : " ");
  }
  if (fclose(output) != 0) {
    free(colors);
    fail("failed closing certificate output");
  }
  free(colors);
  fprintf(stderr, "expanded t=%ld length=%zu -> %s\n", t, length, argv[4]);
  return 0;
}
