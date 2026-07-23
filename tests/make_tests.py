#!/usr/bin/env python3
"""Builds hand-checked test certificates for verifier.c.

This is TEST tooling only. It contains an independent brute-force AP counter
(count_mono_aps) used as a second opinion on every small case, so each test
file's expected verdict is established by an implementation that shares
nothing with verifier.c.
"""
import itertools
import pathlib

HERE = pathlib.Path(__file__).parent


def count_mono_aps(colors, klen):
    """colors: tuple of colors (1-based values) for integers 1..n.
    klen[c] = forbidden AP length for color c. Returns list of violations
    (color, start, diff)."""
    n = len(colors)
    out = []
    for a in range(1, n + 1):
        c = colors[a - 1]
        k = klen[c]
        d = 1
        while a + (k - 1) * d <= n:
            if all(colors[a + j * d - 1] == c for j in range(1, k)):
                out.append((c, a, d))
            d += 1
    return out


def write_cert(name, r, ks, colors, header=""):
    p = HERE / name
    lines = []
    if header:
        for h in header.splitlines():
            lines.append("# " + h)
    lines.append(str(r))
    lines.append(" ".join(str(k) for k in ks))
    lines.append(str(len(colors)))
    lines.append(" ".join(str(c) for c in colors))
    p.write_text("\n".join(lines) + "\n")
    return p


def main():
    # t1: valid W(2,3) > 8, the classic block coloring.
    t1 = (1, 2, 2, 1, 1, 2, 2, 1)
    v = count_mono_aps(t1, {1: 3, 2: 3})
    assert v == [], v
    write_cert("t1_valid_w23_n8.txt", 2, [3, 3], t1,
               "test: VALID  W(2,3) > 8  (comment lines must be ignored)")

    # t2: EXACTLY ONE monochromatic 3-AP, chosen (over n=9..11) to be the
    # most widely spread unique AP so it is genuinely "hidden" — the scan
    # must reach large differences to find it. (No coloring of n>=12 has
    # exactly one violation: past W(2,3)=9 they multiply.)
    best = None
    for n in (9, 10, 11):
        for colors in itertools.product((1, 2), repeat=n):
            v = count_mono_aps(colors, {1: 3, 2: 3})
            if len(v) == 1:
                key = (v[0][2], v[0][1])  # prefer large diff, then late start
                if best is None or key > best[0]:
                    best = (key, colors, v[0])
    _, colors, ap = best
    write_cert("t2_one_hidden_ap.txt", 2, [3, 3], colors,
               f"test: INVALID exactly one mono 3-AP, widely spread: "
               f"color={ap[0]} start={ap[1]} diff={ap[2]}")
    print(f"t2 expected AP: color={ap[0]} start={ap[1]} diff={ap[2]} "
          f"elements={[ap[1] + j * ap[2] for j in range(3)]}")

    # t3: trivially invalid, everything one color.
    write_cert("t3_allsame_n5.txt", 2, [3, 3], (1,) * 5,
               "test: INVALID  all one color")

    # t4/t5: mixed w(3,4;2) — color 1 avoids 3-APs, color 2 avoids 4-APs.
    # Find a valid n=17 witness whose k-swapped reading is INVALID, so the
    # pair also proves the verifier respects the color->length assignment.
    pick = None
    for colors in itertools.product((1, 2), repeat=17):
        if count_mono_aps(colors, {1: 3, 2: 4}):
            continue
        swapped = count_mono_aps(colors, {1: 4, 2: 3})
        if swapped:
            pick = (colors, swapped[0])
            break
    colors, swap_ap = pick
    write_cert("t4_valid_mixed_w34_n17.txt", 2, [3, 4], colors,
               "test: VALID  w(3,4;2) > 17 (Chvatal: w(3,4;2)=18)")
    write_cert("t5_swapped_mixed_n17.txt", 2, [4, 3], colors,
               f"test: INVALID  same coloring, k-list swapped; first "
               f"violation color={swap_ap[0]} start={swap_ap[1]} "
               f"diff={swap_ap[2]}")
    print(f"t5 expected AP: color={swap_ap[0]} start={swap_ap[1]} "
          f"diff={swap_ap[2]}")

    # t6: color out of range -> parse error.
    (HERE / "t6_color_out_of_range.txt").write_text("2\n3 3\n4\n1 2 3 1\n")
    # t7: too few colors -> parse error.
    (HERE / "t7_wrong_count.txt").write_text("2\n3 3\n8\n1 2 2 1 1 2 2\n")
    # t8: k=2 semantics — color 1 may appear at most once.
    t8 = (1, 2, 1)
    v = count_mono_aps(t8, {1: 2, 2: 3})
    assert len(v) == 1 and v[0] == (1, 1, 2), v
    write_cert("t8_k2_semantics.txt", 2, [2, 3], t8,
               "test: INVALID  k=2 means color 1 may appear at most once; "
               "elements 1 and 3 form the 2-AP")
    print("all test files written and cross-checked by brute force")


if __name__ == "__main__":
    main()
