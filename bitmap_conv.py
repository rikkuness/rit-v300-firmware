#!/usr/bin/env python3
import sys
import os
from PIL import Image

if len(sys.argv) < 2:
    print("Usage: python bmp2c.py <input.bmp>")
    sys.exit(1)

infile = sys.argv[1]
basename, _ = os.path.splitext(infile)
outfile = basename + ".c"

# Load and convert to 8-bit grayscale
img = Image.open(infile).convert("L")
w, h = img.size
pixels = list(img.getdata())

with open(outfile, "w") as f:
    f.write(f"// Generated from {infile}\n")
    f.write(f"const unsigned char {basename}_bitmap[{w*h}] = {{\n")
    for i, p in enumerate(pixels):
        f.write(f"{p}, ")
        if (i + 1) % w == 0:
            f.write("\n")
    f.write("};\n")
    f.write(f"const int {basename}_w = {w};\n")
    f.write(f"const int {basename}_h = {h};\n")

print(f"Written {outfile}")
