#ifndef PICO_UOM_H
#define PICO_UOM_H

#include "uart_over_midi.h"

uom_result_t uom_midi_send(const uint8_t *data, int len);
uom_result_t uom_uart_recv(const uint8_t *data, int len);
uom_result_t uom_uart_setup(const uom_uart_config_t *cfg);

#endif
