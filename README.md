# RIT-V300 Firmware

Firmware for the RIT-V300 series of smart terminal by RobCo Industries.

## Target Hardware

This firmware was built and tested on the RP2350, specifically the Adafruit Metro.

## Fonts

Fonts were obtained from https://github.com/viler-int10h/vga-text-mode-fonts

Converted from raw binary into C structs with

```
xxd -i SOME_FONT.F14 > some_font.c
```

Each font is then defined in `fonts.h` and `fonts.c`.

The active system font is referenced from `active_font` and defaults to `Z100_A_F09` (Zenith Z-100, single-dot font, 8x9).

Lines can have additional spacing added with by changing `font_line_spacing` in `fonts.h`.

### ANSI Colours

The terminal supports some ANSI control chars for coloring text. 16 color greyscale only.

Example of sending 8 different highlights over UART.

```sh
for i in {40..47}; do
  printf "=> \x1b[%dmBackground Level %d\x1b[0m\n" "$i" "$((i-40))" > /dev/ttyUSB0
done
```

## Images

## Credits

### Video

All of the composite video work is straight from https://github.com/breakintoprogram/pico-mposite
Some work was done to modify clocking to work properly on the RP2350. Then additional changes made to support fonts and fit things specifically to the CRT I have. I also removed all of the color RGB support as it was not applicable to this application.

A huge amount of knowledge was derived from this detailed breakdown of PAL video timing https://martin.hinner.info/vga/pal.html.
