/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// Modified by @shapoco

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "hub75_driver.h"

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hub75.pio.h"

#define DATA_BASE_PIN 0
#define DATA_N_PINS 6
#define ROWSEL_BASE_PIN 6
#define ROWSEL_N_PINS 4
#define CLK_PIN 11
#define STROBE_PIN 12
#define OEN_PIN 13

#define WIDTH 64
#define HEIGHT 32

static PIO pio = pio0;
static uint sm_data = 0;
static uint sm_row = 1;
static uint data_prog_offs;
static uint row_prog_offs;
static uint32_t img[HEIGHT][WIDTH];

void hub75_init() {
    data_prog_offs = pio_add_program(pio, &hub75_data_rgb888_program);
    row_prog_offs = pio_add_program(pio, &hub75_row_program);
    hub75_data_rgb888_program_init(pio, sm_data, data_prog_offs, DATA_BASE_PIN, CLK_PIN);
    hub75_row_program_init(pio, sm_row, row_prog_offs, ROWSEL_BASE_PIN, ROWSEL_N_PINS, STROBE_PIN);
}

void hub75_clear(uint32_t color) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            img[y][x] = color;
        }
    }
}

void hub75_set_pixel(int x, int y, uint32_t color) {
    if (x < 0 || WIDTH <= x) return;
    if (y < 0 || HEIGHT <= y) return;
    img[y][x] = color;
}

void hub75_send() {
    for (int rowsel = 0; rowsel < (1 << ROWSEL_N_PINS); ++rowsel) {
        for (int bit = 0; bit < 8; ++bit) {
            hub75_data_rgb888_set_shift(pio, sm_data, data_prog_offs, bit);
            for (int x = 0; x < WIDTH; ++x) {
                pio_sm_put_blocking(pio, sm_data, img[rowsel][x]);
                pio_sm_put_blocking(pio, sm_data, img[(1u << ROWSEL_N_PINS) + rowsel][x]);
            }
            pio_sm_put_blocking(pio, sm_data, 0);
            pio_sm_put_blocking(pio, sm_data, 0);
            hub75_wait_tx_stall(pio, sm_data);
            hub75_wait_tx_stall(pio, sm_row);
            pio_sm_put_blocking(pio, sm_row, rowsel | (100u * (1u << bit) << 4));
        }
    }
}