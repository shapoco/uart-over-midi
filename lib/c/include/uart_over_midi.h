#ifndef SERIAL_OVER_MIDI_H
#define SERIAL_OVER_MIDI_H

#include <stdint.h>
#include <stddef.h>

#define UOM_MAX_BYTE_BLK_LEN (7 * 255)
#define UOM_MAX_MIDI_MSG_LEN ((UOM_MAX_BYTE_BLK_LEN / 7) * 8 + 8)

typedef enum {
    UOM_OK = 0,
    UOM_ERR_MIDI_RX_BUFF_OVFL = 1,
    UOM_ERR_MIDI_INVALID_MSG = 2,
    UOM_ERR_INVALID_MARKER = 3,
    UOM_ERR_SYNTAX = 4,
    UOM_ERR_INVALID_PARAM = 5,
    UOM_ERR_INVALID_CTL_CODE = 6,
    UOM_ERR_NO_FUNCTION = 7,
} uom_result_t;

typedef enum {
    UOM_CTL_TRANS_CMD = 0x01,
    UOM_CTL_SETUP_CMD = 0x02,
    UOM_CTL_SETUP_ACK = 0x42,
} uom_control_t;

typedef enum {
    UOM_STS_NONE = 0x00,
} uom_status_t;

typedef enum {
    UOM_PARITY_NONE = 0,
    UOM_PARITY_EVEN = 1,
    UOM_PARITY_ODD = 2,
} uom_parity_t;

typedef struct {
    uint32_t baudrate;
    int data_bits;
    int stop_bits;
    uom_parity_t parity;
} uom_uart_config_t;

typedef uom_result_t (*uom_midi_send_func)(const uint8_t *data, int len);
typedef uom_result_t (*uom_uart_recv_func)(const uint8_t *data, int len);
typedef uom_result_t (*uom_uart_setup_func)(const uom_uart_config_t *cfg);

typedef struct {
    uom_midi_send_func midi_send;
    uom_uart_recv_func uart_recv;
    uom_uart_setup_func uart_setup;
} uom_config_t;

uom_result_t uom_init(const uom_config_t *cfg);
uom_result_t uom_midi_recv(const uint8_t *data, int len);
uom_result_t uom_uart_send(const uint8_t *data, int len);

#endif
