import sys
import os
from PIL import Image

if len(sys.argv) < 2:
    print("Usage: python bmp2c.py <input.bmp>")
    sys.exit(1)

infile = sys.argv[1]
basename, _ = os.path.splitext(os.path.basename(infile))
outfile = basename + ".c"

# Load and convert
img = Image.open(infile).convert("L")  # grayscale
w, h = img.size

# Scale to compensate non square pixels
scale_x = 1.5
scale_y = 1.0

new_w = int(w * scale_x)
new_h = int(h * scale_y)

img = img.resize((new_w, new_h), Image.Resampling.NEAREST)
w, h = img.size

pixels = list(img.getdata())

# Quantize 0–255 -> 0–15
pixels_4bit = [p // 16 for p in pixels]

# Pack 2 pixels per byte (high nibble = left pixel, low nibble = right pixel)
packed = []
for i in range(0, len(pixels_4bit), 2):
    hi = pixels_4bit[i] & 0xF
    lo = pixels_4bit[i + 1] & 0xF if i + 1 < len(pixels_4bit) else 0
    packed.append((hi << 4) | lo)

with open(outfile, "w") as f:
    f.write(f"// Generated from {infile}\n")
    f.write(f"// Image size: {w}x{h}, {len(pixels_4bit)} pixels, {len(packed)} bytes\n")
    f.write(f"const unsigned char {basename}_bitmap[{len(packed)}] = {{\n")
    for i, val in enumerate(packed):
        f.write(f"0x{val:02X}, ")
        if (i + 1) % 16 == 0:
            f.write("\n")
    f.write("};\n")
    f.write(f"const int {basename}_w = {w};\n")
    f.write(f"const int {basename}_h = {h};\n")

print(f"Written {outfile}")
