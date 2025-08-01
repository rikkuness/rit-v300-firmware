
#include <math.h>
#include <stdlib.h>

#include "memory.h"
#include "pico/stdlib.h"

#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"

#include "cvideo.h"
#include "fonts.h"
#include "graphics.h"
#include "terminal.h"

#include "bsp/board_api.h"
#include "tusb.h"

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

char uos[]   = "Unified Operating System";
char robco[] = "RobCo Industries 2074";

// Show a splash screen
void splash() {
  cls(0);

  set_border(col_black);

  print_string((width / 2) - ((strlen(uos) * active_font->char_width) / 2), 24, uos, DEFAULT_BG, DEFAULT_FG);
  print_string((width / 2) - ((strlen(robco) * active_font->char_width) / 2), 180, robco, DEFAULT_BG, DEFAULT_FG);

  sleep_ms(2000);
}

// The main loop
int main() {
  board_init();

  initialise_cvideo(); // Initialise the composite video stuff
  set_mode(4);         // Set video mode to 390 x 240
  splash();            // Display a splash screen
  cls(col_black);

  gpio_init(29);
  gpio_set_dir(29, GPIO_OUT);
  gpio_put(29, 1);

  gpio_init(23);
  gpio_set_dir(23, GPIO_OUT);
  gpio_put(23, 1);

  tuh_init(BOARD_TUH_RHPORT);
  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

  // colour_bars_v();
  while (true) {
    tuh_task(); // PANIC: DMA channel 0 is already claimedd

    terminal_app();
  }
}

// Simple terminal output from UART
//
void terminal_app(void) {
  initialise_terminal(); // Initialise the UART
  cls(DEFAULT_BG);       // Clear the screen
  terminal();            // And do the terminal
}