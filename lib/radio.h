#ifndef __RADIO_H
#define __RADIO_H

#include "stm8s.h"
#include "util.h"
#include "spi.h"
#include "radio_config_Si4463.h"
#include <stdio.h>

#define READ_CMD_BUFF 0x44
#define WRITE_TX_FIFO 0x66
#define START_TX 0x31
#define START_RX 0x32
#define FIFO_INFO 0x15
#define READ_RX_FIFO 0x77
#define GET_INT_STATUS 0x20

#define FIELD_LENGTH 38

void radio_init();

void radio_write_tx_fifo(u8 data[FIELD_LENGTH]);
void radio_start_tx();

void radio_start_rx();
void radio_read_rx_fifo(u8 data[FIELD_LENGTH]);

u8 radio_tx_finished();
u8 radio_rx_finished();

#endif