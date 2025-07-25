#include "fonts.h"

#include "fonts/apri256.c"
#include "fonts/vga8x16.c"
#include "fonts/z100.c"

// Instantiate the font struct
const BitmapFont vga_font_8x16 = {
    .glyphs = VGA8_F16, .char_width = 8, .char_height = 16, .bytes_per_row = 1, .first_char = 0, .last_char = 255};

const BitmapFont apri_font_8x10 = {
    .glyphs = APRI256_F10, .char_width = 8, .char_height = 10, .bytes_per_row = 1, .first_char = 0, .last_char = 255};

const BitmapFont z100_font = {
    .glyphs = Z100_A_F09, .char_width = 8, .char_height = 9, .bytes_per_row = 1, .first_char = 0, .last_char = 255};

// Default active font
BitmapFont *active_font = (BitmapFont *)&z100_font;

void        set_font(const BitmapFont *font) {
  if (font) {
    active_font = (BitmapFont *)font;
  } else {
    active_font = (BitmapFont *)&z100_font;
  }
}