#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pico_uom.h"

#include "pico/stdlib.h"
#include "hardware/uart.h"

#include "bsp/board.h"
#include "tusb.h"

static uart_inst_t *uart = uart0;
#define PIN_UART_TX (0)
#define PIN_UART_RX (1)

static void uom_task(void);
static int uart_read_nonblocking(uart_inst_t *uart, uint8_t *buff, int max_len);

int main(void) {
    stdio_init_all();
    board_init();
    tusb_init();

    uart_init(uart, 115200);
    gpio_set_function(PIN_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(PIN_UART_RX, GPIO_FUNC_UART);

    uom_config_t cfg;
    cfg.midi_send = &uom_midi_send;
    cfg.uart_recv = &uom_uart_recv;
    cfg.uart_setup = &uom_uart_setup;
    uom_init(&cfg);

    while (true) {
        tud_task();
        uom_task();
        sleep_us(100);
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

    while (1) {
        int rx_len = uart_read_nonblocking(uart, rx_buff, RX_BUFF_SIZE);
        if (rx_len == 0) break;
        uom_uart_send(rx_buff, rx_len);
    }
}

static int uart_read_nonblocking(uart_inst_t *uart, uint8_t *buff, int max_len) {
    int n = 0;
    while (uart_is_readable(uart)) {
        buff[n++] = uart_getc(uart);
        if (n >= max_len) break;
    }
    return n;
}

uom_result_t uom_midi_send(const uint8_t *data, int len) {
    tud_midi_stream_write(TX_CABLE_NO, data, len);
    return UOM_OK;
}

uom_result_t uom_uart_recv(const uint8_t *data, int len) {
    uart_write_blocking(uart, data, len);
    return UOM_OK;
}

uom_result_t uom_uart_setup(const uom_uart_config_t *cfg) {
    if (cfg->data_bits < 5 || 8 < cfg->data_bits) return UOM_ERR_INVALID_PARAM;
    if (cfg->stop_bits < 1 || 2 < cfg->stop_bits) return UOM_ERR_INVALID_PARAM;

    uart_parity_t parity;
    switch(cfg->parity) {
    case UOM_PARITY_NONE: parity = UART_PARITY_NONE; break;
    case UOM_PARITY_EVEN: parity = UART_PARITY_EVEN; break;
    case UOM_PARITY_ODD: parity = UART_PARITY_ODD; break;
    default: return UOM_ERR_INVALID_PARAM;
    }

    uint act_baud = uart_set_baudrate(uart, cfg->baudrate);
    uart_set_format(uart, cfg->data_bits, cfg->stop_bits, parity);

    uint baud_min = cfg->baudrate *  975 / 1000;
    uint baud_max = cfg->baudrate * 1025 / 1000;
    if (baud_min <= act_baud && act_baud <= baud_max) {
        return UOM_OK;
    }
    else {
        return UOM_ERR_INVALID_PARAM;
    }
}
