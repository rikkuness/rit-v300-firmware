# RIT-V300 Firware

## Fonts

Fonts were obtained from https://github.com/viler-int10h/vga-text-mode-fonts

Converted from raw binary into C structs with
```
xxd -i SOME_FONT.F14 > some_font.c
```

Each font is then defined in `fonts.h` and `fonts.c`.

The active system font is referenced from `active_font` and defaults to `Z100_A_F09` (Zenith Z-100, single-dot font, 8x9).

Lines can have additional spacing added with by changing `font_line_spacing` in `fonts.h`.