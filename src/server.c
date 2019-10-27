#include "config.h"

u8 rx_buffer[FIELD_LENGTH];
u8 tx_buffer[FIELD_LENGTH];
u32 entropy_offset;

void handle_client_server_hello() {
    ClientServerHello *request = (ClientServerHello *) rx_buffer;
    ServerClientEntropy *response = (ServerClientEntropy *) tx_buffer;

    response->command = SERVER_CLIENT_ENTROPY;
    response->client_id = request->client_id;
    response->entropy = entropy_offset + timer_read();
    radio_write_tx_fifo(tx_buffer);
    radio_start_tx();
    while (!radio_tx_finished());
}

void handle_client_server_unlock() {
    ClientServerUnlock *request = (ClientServerUnlock *) rx_buffer;
    ServerClientStop *response = (ServerClientStop *) tx_buffer;
    response->successful = 1;

    int32_t entropy_difference = (timer_read() + entropy_offset) - request->entropy;
    if (entropy_difference > 5 || entropy_difference < -5) {
        response->successful = 0;
    }

    key.entropy = request->entropy;
    u8 expected_hash[32];
    calc_sha_256(expected_hash, (const void *) (&key), sizeof(key));

    response->command = SERVER_CLIENT_STOP;
    response->client_id = request->client_id;
    for (int a = 0; a < sizeof(expected_hash); a++) {
        if (request->hash[a] != expected_hash[a]) {
            response->successful = 0;
        }
    }

    radio_write_tx_fifo(tx_buffer);
    radio_start_tx();

    // This is the only time we have to write back entropy data to the EEPROM.
    // EEPROM writing is very slow!
    u32 failsafe_entropy = entropy_offset + timer_read();
    eeprom_write(0, (u8 *) &failsafe_entropy, sizeof(u32));

    if (response->successful) {
        printf("Unlock\n");
    }
    while (!radio_tx_finished());
}

void main() {
    clock_init();
    timer_init();
    uart_init();
    spi_init();
    radio_init();
    eeprom_read(0, (u8 *) &entropy_offset, sizeof(u32));
    while (1) {
        radio_start_rx();
        while (!radio_rx_finished());
        radio_read_rx_fifo(rx_buffer);
        switch (rx_buffer[0]) {
            case CLIENT_SERVER_HELLO:
                handle_client_server_hello();
                break;
            case CLIENT_SERVER_UNLOCK:
                handle_client_server_unlock();
                break;
            default:
                break;
        }
    }
}