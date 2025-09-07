#include "cvideo.h"
#include "main.h"

// Test images - vertical bars in 16 colours
void colour_bars_v() {
  const int rectw = width / 16;

  cls(col_black);
  for (int i = 0; i < 16; i++) {
    draw_rect(i * rectw, 0, i * rectw + rectw - 1, height, i, true);
  }
}

// Test images - horizontal bars in 16 colours
void colour_bars_h() {
  const int recth = height / 16;
  for (int i = 0; i < 16; i++) {
    draw_rect(0, i * recth, width, i * recth + recth - 1, i, true);
  }
}
