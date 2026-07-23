#!/usr/bin/env python3
"""Decode a t2_cnf/Kissat model to an unverified complete certificate."""

import sys


def certificate_tokens(path):
    tokens = []
    with open(path, encoding="ascii") as source:
        for line in source:
            if not line.startswith("#"):
                tokens.extend(line.split())
    return tokens


def read_seed(path, n, t):
    seed = [2] * (n + 1)
    if path == "-":
        return seed
    tokens = certificate_tokens(path)
    if len(tokens) < 4:
        raise ValueError("short seed certificate")
    if list(map(int, tokens[:3])) != [2, 3, t]:
        raise ValueError("seed header mismatch")
    seed_n = int(tokens[3])
    if seed_n < 1 or seed_n > n or len(tokens) != 4 + seed_n:
        raise ValueError("bad seed length or trailing tokens")
    for index, token in enumerate(tokens[4:], 1):
        color = int(token)
        if color not in (1, 2):
            raise ValueError("seed color outside [1,2]")
        seed[index] = color
    return seed


def read_model(path):
    assignment = {}
    with open(path, encoding="ascii") as source:
        for line in source:
            if not line.startswith("v"):
                continue
            for token in line.split()[1:]:
                literal = int(token)
                if literal:
                    assignment[abs(literal)] = literal > 0
    return assignment


def main():
    if len(sys.argv) != 6:
        raise SystemExit(
            "usage: t2_decode_model.py N T seed-certificate|- model output"
        )
    n, t = map(int, sys.argv[1:3])
    seed_path, model_path, output_path = sys.argv[3:]
    seed = read_seed(seed_path, n, t)
    assignment = read_model(model_path)
    missing = [variable for variable in range(1, n + 1) if variable not in assignment]
    if missing:
        raise ValueError(f"model omits primary variable {missing[0]}")

    colors = []
    for variable in range(1, n + 1):
        flip = seed_path != "-" and seed[variable] == 1
        logical_color1 = assignment[variable] != flip
        colors.append(1 if logical_color1 else 2)

    with open(output_path, "w", encoding="ascii") as output:
        output.write(
            f"# UNVERIFIED decoded t2_cnf candidate: n={n} t={t} "
            f"seed={seed_path} model={model_path}\n"
        )
        output.write(f"2\n3 {t}\n{n}\n")
        for offset in range(0, n, 60):
            output.write(" ".join(map(str, colors[offset : offset + 60])) + "\n")
    print(f"decoded -> {output_path}")


if __name__ == "__main__":
    main()
