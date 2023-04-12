#include "uart_over_midi.h"

static uom_config_t cfg;
static uint8_t rx_buff[UOM_MAX_MIDI_MSG_LEN];
static uint8_t tx_buff[UOM_MAX_MIDI_MSG_LEN];

typedef enum {
    MIDI_FIRST_BYTE,
    MIDI_MIDDLE_BYTE,
    MIDI_LAST_BYTE,
    MIDI_SYSEX,
} midi_rx_state_t;

static midi_rx_state_t midi_rx_state = MIDI_FIRST_BYTE;
static int midi_rx_len = 0;

static void midi_rx_reset();
static uom_result_t sysex_recv(const uint8_t *data, int len);
static uom_result_t sysex_send(uom_control_t ctl, uom_status_t sts, const uint8_t *data, int len);

uom_result_t uom_init(const uom_config_t *c) {
    cfg = *c;
    return UOM_OK;
}

uom_result_t uom_midi_recv(const uint8_t *data, int len) {
    uom_result_t retval = UOM_OK;
    for (int i = 0; i < len; i++) {
        uint8_t byte = data[i];

        if (midi_rx_len >= UOM_MAX_MIDI_MSG_LEN) {
            retval = UOM_ERR_MIDI_RX_BUFF_OVFL;
            midi_rx_reset();
            break;
        }

        rx_buff[midi_rx_len++] = byte;

        switch (midi_rx_state) {
        case MIDI_FIRST_BYTE: {
            uint8_t high = byte & 0xf0;
            if (byte == 0xf0) {
                // System Exclusive Message
                midi_rx_state = MIDI_SYSEX;
            }
            else if (high == 0x80 || high == 0x90 || high == 0xa0 || high == 0xb0 || high == 0xe0) {
                // 3 Byte Messages
                midi_rx_state = MIDI_MIDDLE_BYTE;
            }
            else if (high == 0xc0 || high == 0xd0 || byte == 0xf1 || byte == 0xf3) {
                // 2 Byte Messages
                midi_rx_state = MIDI_LAST_BYTE;
            }
            else {
                // Single Byte Messages
                midi_rx_reset();
            }
            break;
        }

        case MIDI_MIDDLE_BYTE:
            midi_rx_state = MIDI_LAST_BYTE;
            break;
        
        case MIDI_LAST_BYTE:
            midi_rx_reset();
            break;
        
        case MIDI_SYSEX:
            if (byte == 0xf7) {
                retval = sysex_recv(rx_buff, midi_rx_len);
                midi_rx_reset();
            }
            break;
        }
    }
    return retval;
}

uom_result_t uom_uart_send(const uint8_t *data, int len) {
    sysex_send(UOM_CTL_TRANS_CMD, UOM_STS_NONE, data, len);
}

static void midi_rx_reset() {
    midi_rx_len = 0;
    midi_rx_state = MIDI_FIRST_BYTE;
}

static uom_result_t sysex_recv(const uint8_t *data, int len) {
    if (len < 3) {
        return UOM_ERR_MIDI_INVALID_MSG;
    }
    if (data[0] != 0xf0 || data[len-1] != 0xf7) {
        return UOM_ERR_MIDI_INVALID_MSG;
    }

    int payload_offset = 2;
    uint32_t manufacture_id = data[1];
    if (manufacture_id == 0x00) {
        if (len < 5) {
            return UOM_ERR_MIDI_INVALID_MSG;
        }
        manufacture_id |= ((uint32_t)data[2]) << 8;
        manufacture_id |= (uint32_t)data[3];
        payload_offset = 4;
    }

    int payload_len = len - payload_offset - 1;
    const uint8_t *payload = data + payload_offset;

    if (payload_len < 5) {
        return UOM_ERR_SYNTAX;
    }

    int marker_ok = 
        (payload[0] == 'U') &&
        (payload[1] == 'o') &&
        (payload[2] == 'M');
    if (!marker_ok) {
        return UOM_ERR_INVALID_MARKER;
    }

    uom_control_t control_byte = (uom_control_t)payload[3];
    uom_status_t status_byte = (uom_status_t)payload[4];

    int phase = 0;
    uint8_t tmp = 0;
    int n = 0;
    for (int i = 5; i < payload_len; i++) {
        uint8_t b = payload[i];
        switch (phase) {
        case 0: tmp = b & 0x7f; break;
        case 1: tx_buff[n++] = ((b << 7) & 0x80) | tmp; tmp = (b >> 1) & 0x3f; break;
        case 2: tx_buff[n++] = ((b << 6) & 0xc0) | tmp; tmp = (b >> 2) & 0x1f; break;
        case 3: tx_buff[n++] = ((b << 5) & 0xe0) | tmp; tmp = (b >> 3) & 0x0f; break;
        case 4: tx_buff[n++] = ((b << 4) & 0xf0) | tmp; tmp = (b >> 4) & 0x07; break;
        case 5: tx_buff[n++] = ((b << 3) & 0xf8) | tmp; tmp = (b >> 5) & 0x03; break;
        case 6: tx_buff[n++] = ((b << 2) & 0xfc) | tmp; tmp = (b >> 6) & 0x01; break;
        case 7: tx_buff[n++] = ((b << 1) & 0xfe) | tmp;
        }
        phase = (phase + 1) & 0x7;
    }
    
    switch (control_byte) {
    case UOM_CTL_TRANS_CMD:
        if (cfg.uart_recv != NULL) {
            return cfg.uart_recv(tx_buff, n);
        }
        else {
            return UOM_ERR_NO_FUNCTION;
        }
        
    case UOM_CTL_SETUP_CMD: {
        uom_result_t ret = UOM_OK;
        if (cfg.uart_setup == NULL) {
            ret = UOM_ERR_NO_FUNCTION;
        }
        if (n == 8) {
            uom_uart_config_t uart_cfg;
            uart_cfg.baudrate =
                ((uint32_t)tx_buff[0]) |
                (((uint32_t)tx_buff[1]) << 8) |
                (((uint32_t)tx_buff[2]) << 16) |
                (((uint32_t)tx_buff[3]) << 24);
            uart_cfg.data_bits = tx_buff[4];
            uart_cfg.stop_bits = tx_buff[5];
            uart_cfg.parity = (uom_parity_t)tx_buff[6];
            ret = cfg.uart_setup(&uart_cfg);
        }
        else {
            ret = UOM_ERR_SYNTAX;
        }
        
        uint8_t ack_code = (uint8_t)ret;
        sysex_send(UOM_CTL_SETUP_ACK, UOM_STS_NONE, &ack_code, 1);
        
        return ret;
    }
    
    case UOM_CTL_SETUP_ACK:
        return UOM_OK;
    
    default:
        return UOM_ERR_INVALID_CTL_CODE;
    }
}

static uom_result_t sysex_send(uom_control_t ctl, uom_status_t sts, const uint8_t *data, int len) {
    if (cfg.midi_send == NULL) {
        return UOM_ERR_NO_FUNCTION;
    }

    int n = 0;
    tx_buff[n++] = 0xf0;
    tx_buff[n++] = 0x7d;
    tx_buff[n++] = 'U';
    tx_buff[n++] = 'o';
    tx_buff[n++] = 'M';
    tx_buff[n++] = (uint8_t)ctl;
    tx_buff[n++] = (uint8_t)sts;

    int phase = 0;
    uint8_t tmp = 0;
    for (int i = 0; i < len; i++) {
        uint8_t b = data[i];
        switch (phase) {
        case 0: tx_buff[n++] = b & 0x7f; tmp = (b >> 7) & 0x01; break;
        case 1: tx_buff[n++] = ((b << 1) & 0x7e) | tmp; tmp = (b >> 6) & 0x03; break;
        case 2: tx_buff[n++] = ((b << 2) & 0x7c) | tmp; tmp = (b >> 5) & 0x07; break;
        case 3: tx_buff[n++] = ((b << 3) & 0x78) | tmp; tmp = (b >> 4) & 0x0f; break;
        case 4: tx_buff[n++] = ((b << 4) & 0x70) | tmp; tmp = (b >> 3) & 0x1f; break;
        case 5: tx_buff[n++] = ((b << 5) & 0x60) | tmp; tmp = (b >> 2) & 0x3f; break;
        case 6: tx_buff[n++] = ((b << 6) & 0x40) | tmp; tx_buff[n++] = (b >> 1) & 0x7f; break;
        }
        if (phase < 6) {
            phase++;
        }
        else {
            phase = 0;
        }
    }

    if (phase != 0) {
        tx_buff[n++] = tmp;
    }

    tx_buff[n++] = 0xf7;

    return cfg.midi_send(tx_buff, n);
}
