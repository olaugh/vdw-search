CC ?= cc
CXX ?= c++
PYTHON ?= python3

CPPFLAGS ?= -D_POSIX_C_SOURCE=200809L
CFLAGS ?= -O3 -std=c11 -Wall -Wextra
LDFLAGS ?=
LDLIBS ?=

PROGRAMS = verifier aks_expand gen_residue sls t2_sls t2_cnf \
	cnf_linear cnf_cyclic

.PHONY: all check test reproduce-published verify-certificates \
	verify-hashes clean

all: $(PROGRAMS)

$(PROGRAMS): %: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< $(LDFLAGS) $(LDLIBS) -o $@

CADICAL_PREFIX ?= $(shell brew --prefix cadical 2>/dev/null)

t2_cadical: t2_cadical.c
	@test -n "$(CADICAL_PREFIX)" || { \
		echo "Set CADICAL_PREFIX to the CaDiCaL installation prefix."; \
		exit 2; \
	}
	$(CC) $(CPPFLAGS) $(CFLAGS) -I$(CADICAL_PREFIX)/include -c $< -o $@.o
	$(CXX) $@.o $(CADICAL_PREFIX)/lib/libcadical.a -pthread $(LDFLAGS) -o $@

test: verifier t2_sls
	$(PYTHON) tests/make_tests.py
	sh tests/run_verifier_tests.sh ./verifier
	./t2_sls --self-test

verify-certificates: verifier
	sh scripts/verify_certificates.sh ./verifier

verify-hashes:
	sh scripts/verify_hashes.sh

reproduce-published: verifier aks_expand gen_residue
	sh scripts/reproduce_published.sh

check: all
	$(MAKE) test
	$(MAKE) reproduce-published
	$(MAKE) verify-certificates
	$(MAKE) verify-hashes

clean:
	rm -f $(PROGRAMS) t2_cadical t2_cadical.o
