CC ?= cc
CXX ?= c++
PYTHON ?= python3
TECTONIC ?= tectonic
PDF_BUILD_DIR ?= tmp/pdfs
PDF_OUTPUT_DIR ?= output/pdf

CPPFLAGS ?= -D_POSIX_C_SOURCE=200809L
CFLAGS ?= -O3 -std=c11 -Wall -Wextra
LDFLAGS ?=
LDLIBS ?=

PROGRAMS = verifier aks_expand gen_residue sls t2_sls t2_cnf \
	cnf_linear cnf_cyclic

.PHONY: all check test reproduce-published verify-certificates \
	verify-hashes paper clean

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

paper: paper/main.tex paper/references.bib
	mkdir -p $(PDF_BUILD_DIR) $(PDF_OUTPUT_DIR)
	cp paper/references.bib $(PDF_BUILD_DIR)/references.bib
	$(TECTONIC) paper/main.tex --outdir $(PDF_BUILD_DIR) \
		--keep-logs --keep-intermediates
	cp $(PDF_BUILD_DIR)/main.pdf $(PDF_OUTPUT_DIR)/vdw-search-paper.pdf

clean:
	rm -f $(PROGRAMS) t2_cadical t2_cadical.o
