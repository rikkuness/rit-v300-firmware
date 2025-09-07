#pragma once

#define DEFAULT_FG  0x0f
#define DEFAULT_BG  0x00

#define UART_RX_PIN 8
#define UART_TX_PIN 9
// #define UART_SPEED 1200
#define UART_SPEED   115200

#define ANSI_BUF_MAX 16

void initialise_terminal(void);
void terminal(void);
