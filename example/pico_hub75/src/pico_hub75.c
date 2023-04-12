#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico_hub75.h"
#include "hub75_driver.h"

#include "pico/stdlib.h"

#include "bsp/board.h"
#include "tusb.h"

static void uom_task(void);

int main(void) {
    stdio_init_all();
    board_init();
    tusb_init();
    hub75_init();

    uom_config_t cfg;
    cfg.midi_send = &uom_midi_send;
    cfg.uart_recv = &uom_uart_recv;
    cfg.uart_setup = NULL;
    uom_init(&cfg);

    while (true) {
        tud_task();
        uom_task();
        hub75_send();
    }
    return 0;
}

void tud_mount_cb(void) {
}

void tud_umount_cb(void) {
}

void tud_suspend_cb(bool remote_wakeup_en) {
}

void tud_resume_cb(void) {
}

static const int RX_BUFF_SIZE = 64;
static const int TX_CABLE_NO = 0;

static void uom_task(void) {
    uint8_t rx_buff[RX_BUFF_SIZE];

    while (tud_midi_available()) {
        int rx_len = tud_midi_stream_read(rx_buff, RX_BUFF_SIZE);
        uom_midi_recv(rx_buff, rx_len);
    }
}

uom_result_t uom_midi_send(const uint8_t *data, int len) {
    tud_midi_stream_write(TX_CABLE_NO, data, len);
}

uom_result_t uom_uart_recv(const uint8_t *data, int len) {
    if (len < 1) return;
    switch(data[0]) {
    case 1:
        if (len < 5) return;
        hub75_set_pixel(
            data[1], data[2],
            ((uint32_t)data[5] << 16) | ((uint32_t)data[4] << 8) | data[3]);
        break;
    case 2: {
        hub75_clear(0);
        break;
    }
    }

}
