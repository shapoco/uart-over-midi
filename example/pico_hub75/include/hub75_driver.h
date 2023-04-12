#ifndef HUB75_DRIVER_H
#define HUB75_DRIVER_H

#include <stdint.h>

void hub75_init();
void hub75_clear(uint32_t color);
void hub75_set_pixel(int x, int y, uint32_t color);
void hub75_send();

#endif
