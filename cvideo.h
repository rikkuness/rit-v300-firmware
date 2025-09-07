#pragma once

// Clock frequency of state machine for PIO handling sync
#define piofreq_0 6.25f

// These are approx 9.8% faster than RP2040
#define piofreq_1_256 7.689f
#define piofreq_1_320 6.151f
#define piofreq_1_392 4.98f
#define piofreq_1_420 4.5f
#define piofreq_1_640 3.075f

#define sm_sync       0 // State machine number in the PIO for the sync data
#define sm_data       1 // State machine number in the PIO for the pixel data

// Start colour; for monochrome version this relates to black level voltage
#define colour_base 0x10
#define colour_max  0x0f // Last available colour
#define HSLO        0x0001
#define HSHI        0x000d
#define VSLO        HSLO
#define VSHI        HSHI
#define BORD        0x8000
#define gpio_base   3
#define gpio_count  5
#define sync_gpio   7

extern unsigned char *bitmap;

extern int            width;
extern int            height;

int                   initialise_cvideo(void);
int                   set_mode(int mode);

void cvideo_configure_pio_dma(PIO pio, uint sm, uint dma_channel, dma_channel_transfer_size transfer_size, size_t buffer_size,
                              irq_handler_t handler);

void cvideo_pio_handler(void);
void cvideo_dma_handler(void);

void wait_vblank(void);
void set_border(unsigned char colour);
