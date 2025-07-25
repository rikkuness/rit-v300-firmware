//
// Title:	        Pico-mposite Terminal Emulation
// Description:		Simple terminal emulation
// Author:	        Dean Belfield
// Created:	        19/02/2022
// Last Updated:	03/03/2022
//
// Modinfo:
// 03/03/2022:      Added colour
#include <stdlib.h>

#include "pico/stdlib.h"

#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pio.h"
#include "hardware/uart.h"

#include "cvideo.h"
#include "fonts.h"
#include "graphics.h"
#include "terminal.h"

int  terminal_x;
int  terminal_y;

void initialise_terminal(void) {
  uart_init(uart0, UART_SPEED);
  gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART); // RX
  gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART); // TX
}

// Handle carriage returns
void cr(void) {
  terminal_x = 0;
  terminal_y += active_font->char_height + font_line_spacing;
  if (terminal_y >= height) {
    terminal_y -= active_font->char_height + font_line_spacing;
    scroll_up(DEFAULT_BG, (active_font->char_height + font_line_spacing));
  }
}

// Advance one character position
void fs(void) {
  terminal_x += active_font->char_width;
  if (terminal_x >= width) {
    cr();
  }
}

// Backspace
void bs(void) {
  terminal_x -= active_font->char_width;
  if (terminal_x < 0) {
    terminal_x = 0;
  }
}

// The terminal loop
void terminal(void) {
  terminal_x = 0;
  terminal_y = 0;

  // Monochrome colors
  uint16_t default_fg                               = DEFAULT_FG; // white
  uint16_t default_bg                               = DEFAULT_BG; // black

  uint16_t col_fg                                   = default_fg;
  uint16_t col_bg                                   = default_bg;

  enum { STATE_NORMAL, STATE_ESC, STATE_CSI } state = STATE_NORMAL;
  char ansi_buf[8];
  int  ansi_len = 0;

  while (true) {
    // Cursor indicator
    print_char(terminal_x, terminal_y, '_', col_bg, col_fg);

    char c = uart_getc(uart0); // Blocking read

    // Clear old cursor
    print_char(terminal_x, terminal_y, ' ', col_bg, col_fg);

    switch (state) {
    case STATE_NORMAL:
      if (c == 0x1B) {
        state = STATE_ESC;
      } else if (c >= 32) {
        print_char(terminal_x, terminal_y, c, col_bg, col_fg);
        fs();
      } else {
        switch (c) {
        case 0x08:
          bs();
          break;
        case 0x0D:
          cr();
          break;
        case 0x03:
          return;
        default:
          break;
        }
      }
      break;

    case STATE_ESC:
      if (c == '[') {
        state    = STATE_CSI;
        ansi_len = 0;
      } else {
        state = STATE_NORMAL;
      }
      break;

    case STATE_CSI:
      if ((c >= '0' && c <= '9') || c == ';') {
        if (ansi_len < sizeof(ansi_buf) - 1) {
          ansi_buf[ansi_len++] = c;
        }
      } else {
        ansi_buf[ansi_len] = '\0';

        if (c == 'm') {
          // Simple SGR code parsing (only one at a time for now)
          char *p = ansi_buf;
          while (*p) {
            int code = atoi(p);
            switch (code) {
            case 0: // Reset
              col_fg = default_fg;
              col_bg = default_bg;
              break;
            case 7: // Reverse video
            {
              uint16_t tmp = col_fg;
              col_fg       = col_bg;
              col_bg       = tmp;
            } break;
            case 1: // Bold → light gray
              col_fg = GRAY16(12);
              break;
            case 2: // Dim → dark gray
              col_fg = GRAY16(4);
              break;
            // Optional: ANSI 30–37 foreground greyscale
            case 30:
              col_fg = GRAY16(0);
              break;
            case 31:
              col_fg = GRAY16(2);
              break;
            case 32:
              col_fg = GRAY16(4);
              break;
            case 33:
              col_fg = GRAY16(6);
              break;
            case 34:
              col_fg = GRAY16(8);
              break;
            case 35:
              col_fg = GRAY16(10);
              break;
            case 36:
              col_fg = GRAY16(12);
              break;
            case 37:
              col_fg = GRAY16(15);
              break;
            // Optional: ANSI 40–47 background greyscale
            case 40:
              col_bg = GRAY16(0);
              break;
            case 41:
              col_bg = GRAY16(2);
              break;
            case 42:
              col_bg = GRAY16(4);
              break;
            case 43:
              col_bg = GRAY16(6);
              break;
            case 44:
              col_bg = GRAY16(8);
              break;
            case 45:
              col_bg = GRAY16(10);
              break;
            case 46:
              col_bg = GRAY16(12);
              break;
            case 47:
              col_bg = GRAY16(15);
              break;
            }

            // Skip to next
            while (*p && *p != ';')
              p++;
            if (*p == ';') p++;
          }
        }

        state = STATE_NORMAL;
      }
      break;
    }
  }
}
