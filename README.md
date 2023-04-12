# UoM: UART over MIDI

## Overview

A protocol for transferring arbitrary byte sequences using MIDI.

## Protocol

### Basic Packet Format

![](img/basic_packet_format.png)

### Payload Encoding

![](img/payload_encoding.png)

### Control Code Definition and Payload Format

![](img/ctl_code_def_and_payload_fmt.png)

### Error Codes

|Code|Mnemonic|Description|
|:--:|:--|:--|
|0|UOM_OK|No Error|
|1|UOM_ERR_MIDI_RX_BUFF_OVFL|MIDI Message Too Long|
|2|UOM_ERR_MIDI_INVALID_MSG|Invalid MIDI Message|
|3|UOM_ERR_INVALID_MARKER|Invalid UoM Marker|
|4|UOM_ERR_SYNTAX|Invalid UoM Packet Format|
|5|UOM_ERR_INVALID_PARAM|Invalid Parameter|
|6|UOM_ERR_INVALID_CTL_CODE|Unknown Control Code|
|7|UOM_ERR_NO_FUNCTION|Function Not Supported|

----