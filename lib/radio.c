/*
 * Pin# | Function          | board pin/signal
 * 1    | PD4               | Si4463 p1 - SDN - Shutdown Pin
 * 12   | PB4               | Si4463 p9  - GPIO0
 * 13   | PC3               | Si4463 p10 - GPIO1 - CTS
 * 14   | PC4               | Si4463 p11 - nIRQ - Interrupt (active low)
 *
 * 15   | PC5/SPI_SCK       | Si4463 p12 - SPI - SCLK
 * 16   | PC6/SPI_MOSI      | Si4463 p14 - SPI - SDI
 * 17   | PC7/SPI_MISO      | Si4463 p13 - SPI - SDO
 * 19   | PD2               | Si4463 p15 - SPI - nSEL
 *
 *
 * 2    | PD5/UART1_TX      | header TXD pad via level shifter
 * 3    | PD6/UART1_RX      | header RXD pad via level shifter
 *
 * 4    | nRST              | reset test point nearest header RXD pad
 * 18   | PD1/SWIM          | SWIM test point nearest header TXD pad
 *
 * 11   | PB5               | header SET pad via level shifter
 */

#include <radio.h>

#define radio_begin() GPIOD->ODR &= ~(1 << 2)
#define radio_end() while (SPI->SR & SPI_SR_BSY); GPIOD->ODR |= (1 << 2)
#define radio_cts_wait() while ((GPIOC->IDR & (1 << 3)) == 0)
#define radio_nIRQ_active() ((GPIOC->IDR & (1 << 4)) == 0)

u8 rx_data_ready = 0;
u8 tx_data_sent = 0;

const u8 radio_configuration_data[] = RADIO_CONFIGURATION_DATA_ARRAY;

void radio_write_command(u8 command, const u8 * parameters, u8 len) {
    radio_cts_wait();
    radio_begin();
    spi_write(command);
    spi_write_buffer(parameters, len);
    radio_end();
}

void radio_read_response(u8 *response, u8 len) {
    radio_cts_wait();
    radio_begin();
    spi_write(READ_CMD_BUFF);
    while (len--) {
        *response++ = spi_read();
    }
    radio_end();
}

void radio_init() {
    // Radio shutdown pin (SDN).
    // Set PD4 to push-pull fast mode.
    GPIOD->DDR |= (1 << 4);
    GPIOD->CR1 |= (1 << 4);
    GPIOD->CR2 |= (1 << 4);

    // Set the SDN pin to high to shutdown radio.
    GPIOD->ODR |= (1 << 4);

    // Radio chip select (nSEL).
    // Set PD2 to push-pull fast mode.
    GPIOD->DDR |= (1 << 2);
    GPIOD->CR1 |= (1 << 2);
    GPIOD->CR2 |= (1 << 2);

    // Power on reset pin (GPIO0, POR).
    // PB4 - Floating without interrupt - pullup off.
    GPIOB->DDR &= ~(1 << 4);
    GPIOB->CR1 &= ~(1 << 4);
    GPIOB->CR2 &= ~(1 << 4);

    // Clear to send pin (GPIO1, CTS).
    // PC3 - Floating without interrupt - pullup off.
    GPIOC->DDR &= ~(1 << 3);
    GPIOC->CR1 &= ~(1 << 3);
    GPIOC->CR2 &= ~(1 << 3);

    // Interrupt pin (nIRQ).
    // PC4 - Floating without interrupt - pullup off.
    GPIOC->DDR &= ~(1 << 4);
    GPIOC->CR1 &= ~(1 << 4);
    GPIOC->CR2 &= ~(1 << 4);

    // set the SDN pin to low to enable radio.
    GPIOD->ODR &= ~(1 << 4);

    // Wait until the reset is complete (POR line goes high).
    while ((GPIOB->IDR & (1 << 4)) == 0);

    // Send the radio configuration data.
    const u8 * current_command = radio_configuration_data;
    while (*current_command > 0) {
        // Setup command parameters. Structure is {command_length, command, ...parameters} {...}
        const u8 command = *(current_command + 1);
        const u8 *parameters = current_command + 2;
        const u8 command_length = *current_command;
        radio_write_command(command, parameters, command_length - 1);
        current_command += *current_command + 1;
    }
}

void radio_read_interrupts() {
    u8 parameters[] = {0, 0, 0};
    radio_write_command(GET_INT_STATUS, parameters, sizeof(parameters));

    u8 get_int_response[9];
    radio_read_response(get_int_response, sizeof(get_int_response));

    // PACKET_SENT_PEND
    if (get_int_response[3] & (1 << 5)) {
        tx_data_sent = 1;
    }

    // PACKET_RX_PEND
    if (get_int_response[3] & (1 << 4)) {
        rx_data_ready = 1;
    }
}

u8 radio_tx_finished() {
    if (radio_nIRQ_active()) {
        radio_read_interrupts();
    }
    return tx_data_sent;
}

u8 radio_rx_finished() {
    if (radio_nIRQ_active()) {
        radio_read_interrupts();
    }
    return rx_data_ready;
}


void radio_write_tx_fifo(u8 data[FIELD_LENGTH]) {
    radio_write_command(WRITE_TX_FIFO, data, FIELD_LENGTH);
    tx_data_sent = 0;
}

void radio_start_tx() {
    u8 start_tx_parameters[] = {
            0, // Channel
            0, // Condition (TX_COMPLETE_STATE, RETRANSMIT, START)
            0, // TX_LEN [12:8]
            0  // TX_LEN[0:7]
    };
    radio_write_command(START_TX, start_tx_parameters, sizeof(start_tx_parameters));
}

void radio_start_rx() {
    u8 start_rx_parameters[] = {
            0, // Channel
            0, // Start condition (immediate)
            0, // RX_LEN Lo
            0, // RX_LEN Hi
            0x08, // Next state rx timeout (Re-arm)
            0x08, // Next state rx valid (Re-arm)
            0x08  // Next state rx invalid (Re-arm)
    };
    radio_write_command(START_RX, start_rx_parameters, sizeof(start_rx_parameters));
}

void radio_read_rx_fifo(u8 data[FIELD_LENGTH]) {
    radio_begin();
    spi_write(READ_RX_FIFO);
    for (u8 a = 0; a < FIELD_LENGTH; a++) {
        data[a] = spi_read();
    }
    radio_end();
    rx_data_ready = 0;
}