#!/usr/bin/env python3
import sys, argparse, os
from svgpathtools import svg2paths

def emit_path(path, name, scale, minx, miny):
    cmds = []
    pts = []

    for seg in path:
        cname = seg.__class__.__name__
        if cname == "Line":
            cmds.append("VG_LINE_TO")
            pts.append(((seg.end.real - minx) * scale,
                        (seg.end.imag - miny) * scale))
        elif cname == "CubicBezier":
            cmds.append("VG_CUBIC_TO")
            pts.extend([
                ((seg.control1.real - minx) * scale,
                 (seg.control1.imag - miny) * scale),
                ((seg.control2.real - minx) * scale,
                 (seg.control2.imag - miny) * scale),
                ((seg.end.real - minx) * scale,
                 (seg.end.imag - miny) * scale),
            ])
        elif cname == "QuadraticBezier":
            cmds.append("VG_QUAD_TO")
            pts.extend([
                ((seg.control.real - minx) * scale,
                 (seg.control.imag - miny) * scale),
                ((seg.end.real - minx) * scale,
                 (seg.end.imag - miny) * scale),
            ])
        elif cname == "Move":
            cmds.append("VG_MOVE_TO")
            pts.append(((seg.end.real - minx) * scale,
                        (seg.end.imag - miny) * scale))
        else:
            print(f"// Skipping unsupported segment: {seg}", file=sys.stderr)

    return cmds, [(int(round(x)), int(round(y))) for x, y in pts]

def main():
    ap = argparse.ArgumentParser(description="Convert SVG paths into C arrays for VGPath")
    ap.add_argument("svgfile", help="Input SVG file")
    ap.add_argument("--max-width", type=float, default=None,
                    help="Scale output so bounding box width = this many pixels")
    args = ap.parse_args()

    basename = os.path.splitext(os.path.basename(args.svgfile))[0]
    cfile = basename + "_vectors.c"
    hfile = basename + "_vectors.h"

    paths, attrs = svg2paths(args.svgfile)

    # Compute bounding box
    xs, ys = [], []
    for p in paths:
        for seg in p:
            xs += [seg.start.real, seg.end.real]
            ys += [seg.start.imag, seg.end.imag]
    minx, maxx = min(xs), max(xs)
    miny, maxy = min(ys), max(ys)
    width = maxx - minx
    scale = 1.0
    if args.max_width and width > 0:
        scale = args.max_width / width

    all_defs = []  # (name, cmds, pts)
    for i, path in enumerate(paths):
        name = f"{basename}{i}"
        cmds, pts = emit_path(path, name, scale, minx, miny)
        all_defs.append((name, cmds, pts))

    # Write .c file
    with open(cfile, "w") as f:
        f.write('#include "vector.h"\n')
        f.write(f'#include "{hfile}"\n\n')
        for name, cmds, pts in all_defs:
            f.write(f"// Path: {name}\n")
            f.write(f"VGCmd {name}_cmds[] = {{ {', '.join(cmds)} }};\n")
            f.write(
                f"VGPoint {name}_pts[] = {{ "
                + ", ".join(f"{{{x},{y}}}" for x, y in pts)
                + " };\n\n"
            )

    # Write .h file
    with open(hfile, "w") as f:
        f.write("#pragma once\n")
        f.write('#include "vector.h"\n\n')
        for name, cmds, pts in all_defs:
            f.write(f"extern VGCmd {name}_cmds[{len(cmds)}];\n")
            f.write(f"extern VGPoint {name}_pts[{len(pts)}];\n\n")

    print(f"Generated {cfile} and {hfile}")

if __name__ == "__main__":
    main()
