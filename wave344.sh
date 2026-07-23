#!/bin/bash
# ILS-style outer loop: run, capture best state, reseed from it with light
# perturbation; any SOLVED output is left for the driver to verify.
cd /Users/john/sources/vdw
id=$1; seedarg=$2; extra=$3; rng=$((400 + id))
seed="$seedarg"
for round in 1 2 3 4 5 6; do
  out=runs/v2_344_${id}_r${round}.txt
  best=runs/v2b_344_${id}_r${round}.txt
  if ./sls -n 344 -r 7 -m ddfw -f 150000000 -s $((rng * 97 + round)) \
      -i "$seed" $extra -b "$best" -o "$out" 2>> runs/v2_344_${id}.log; then
    echo "SOLVED round=$round" >> runs/v2_344_${id}.log
    exit 0
  fi
  seed="$best"; extra="-P 2"   # reseed from best, light kick
done
