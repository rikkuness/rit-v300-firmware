//
// Title:	        Pico-mposite Terminal Emulation
// Author:	        Dean Belfield
// Created:	        19/02/2022
// Last Updated:	03/03/2022
//
// Modinfo:
// 03/03/2022:      Added colour

#pragma once

#define col_terminal_bg 0x00
#define col_terminal_fg 0x0f
#define col_terminal_border 0x04
#define col_terminal_cursor 0x0f

#define terminal_rx_pin 0
#define terminal_tx_pin 1
#define terminal_baud 1200

void initialise_terminal(void);
void terminal(void);