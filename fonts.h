
#pragma once

#include <stdint.h>

#define font_line_spacing 1

// Define the BitmapFont structure
typedef struct {
  const uint8_t *glyphs; // Pointer to glyph data
  int char_width;        // Width in pixels (usually 8)
  int char_height;       // Height in pixels (e.g. 16)
  int bytes_per_row;     // Usually 1 for <=8 width
  int first_char;        // Usually 0 or 32
  int last_char;         // Usually 255 or 127
} BitmapFont;

extern const BitmapFont vga_font_8x16;
extern const BitmapFont apri_font_8x10;
extern const BitmapFont z100_font;

// Global active font
extern BitmapFont *active_font;

// Font setter
void set_font(const BitmapFont *font);
