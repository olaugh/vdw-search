#!/usr/bin/env python3
# glue: DIMACS model + emitter seed -> cert file (verifier judges)
import sys
n, r = int(sys.argv[1]), int(sys.argv[2])
seedfile, modelfile, outfile = sys.argv[3], sys.argv[4], sys.argv[5]
seed = [0]*(n+1)
if seedfile != "-":
    toks=[]
    for line in open(seedfile):
        if line.startswith('#'): continue
        toks += line.split()
    sr=int(toks[0]); sn=int(toks[sr+1])
    for i,x in enumerate(toks[sr+2:sr+2+min(sn,n)]): seed[i+1]=int(x)
lits=set()
for line in open(modelfile):
    if line.startswith('v'):
        for t in line.split()[1:]:
            v=int(t)
            if v: lits.add(v)
cols=[0]*(n+1)
for i in range(1,n+1):
    for c in range(1,r+1):
        v=(i-1)*r+c
        raw = v in lits
        flip = (seed[i]==c)
        if raw ^ flip:
            cols[i]=c
            break
assert all(cols[1:]), "undecoded position"
with open(outfile,"w") as f:
    f.write(f"# decoded kissat model (seed={seedfile})\n{r}\n"+" ".join(["3"]*r)+f"\n{n}\n"+" ".join(map(str,cols[1:]))+"\n")
print("decoded ->", outfile)
