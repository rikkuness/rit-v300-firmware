#pragma once

#define col_black 0x00
#define col_grey  0x0C
#define col_white 0x0F

void splash(char *extra);
void terminal_app(void);
int  main(void);

// USB host power enable pin
#define USBH_POWER_EN 29

// USB host pins
#define USBH_DPLUS  32
#define USBH_DMINUS 33
