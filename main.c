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
#include "main.h"
#include "tusb.h"

#include "logo_vt_vectors.h"
#include "vector.h"

void draw_logo(void) {
  // TODO: It's kinda jacked up, thanks ChatGPT
  VGPath path;
  vg_path_init(&path,
               logo_vt0_cmds,                                    // cmd_buf
               sizeof(logo_vt0_cmds) / sizeof(logo_vt0_cmds[0]), // cmd_cap
               logo_vt0_pts,                                     // pt_buf
               sizeof(logo_vt0_pts) / sizeof(logo_vt0_pts[0])    // pt_cap
  );

  VGFlatten flt = {
      1, // curve_flatness_sq
      10 // subdiv_limit
  };

  vg_fill_path(&path, 0x7, &flt);
  vg_stroke_path(&path, 0xa, NULL, &flt);
}

char uos[] = "Unified Operating System";
char rob[] = "RobCo Industries 2074";

// Show a splash screen
void splash() {
  cls(0);

  set_border(col_black);

  print_string((width / 2) - ((strlen(uos) * active_font->char_width) / 2), 24, uos, DEFAULT_BG, DEFAULT_FG);
  print_string((width / 2) - ((strlen(rob) * active_font->char_width) / 2), 180, rob, DEFAULT_BG, DEFAULT_FG);

  draw_logo();

  sleep_ms(2000 * 20);
}

// The main loop
int main() {
  // This totally fucks up the graphics
  // board_init();

  initialise_cvideo(); // Initialise the composite video stuff
  set_mode(4);         // Set video mode to 390 x 240
  splash();            // Display a splash screen
  cls(col_black);

  // USB host power
  gpio_init(29);
  gpio_set_dir(29, GPIO_OUT);
  gpio_put(29, 1);

  // On board LED
  gpio_init(23);
  gpio_set_dir(23, GPIO_OUT);

  // This breaks GPIO RX
  // tuh_init(BOARD_TUH_RHPORT);
  // if (board_init_after_tusb) {
  // board_init_after_tusb();
  // }

  // colour_bars_v();
  while (true) {
    // tuh_task(); // PANIC: DMA channel 0 is already claimed

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