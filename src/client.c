#include "config.h"

u8 rx_buffer[FIELD_LENGTH];
u8 tx_buffer[FIELD_LENGTH];

int wait_for_rx_with_timeout() {
    radio_start_rx();
    u32 start = timer_read_100ms();
    while (!radio_rx_finished()) {
        if((timer_read_100ms() - start) >= 2) {
            return -1;
        }
    }
    return 0;
}

void main() {
    clock_init();
    timer_init();
    uart_init();
    spi_init();
    radio_init();

    while (1) {
        ClientServerHello *entropy_request = (ClientServerHello *) tx_buffer;
        ClientServerUnlock *unlock_request = (ClientServerUnlock *) tx_buffer;
        ServerClientEntropy *entropy_response = (ServerClientEntropy *) rx_buffer;
        ServerClientStop *stop_response = (ServerClientStop *) rx_buffer;

        entropy_request->command = CLIENT_SERVER_HELLO;
        entropy_request->client_id = CLIENT_ID;

        radio_write_tx_fifo(tx_buffer);
        radio_start_tx();
        while (!radio_tx_finished());

        if (wait_for_rx_with_timeout() < 0) {
            continue;
        }
        radio_read_rx_fifo(rx_buffer);

        if (entropy_response->command != SERVER_CLIENT_ENTROPY) {
            continue;
        }
        if (entropy_response->client_id != CLIENT_ID) {
            continue;
        }

        key.entropy = entropy_response->entropy;
        calc_sha_256(unlock_request->hash, (void *)&key, sizeof(Key));
        unlock_request->command = CLIENT_SERVER_UNLOCK;
        unlock_request->entropy = entropy_response->entropy;

        radio_write_tx_fifo(tx_buffer);
        radio_start_tx();
        while(!radio_tx_finished());

        if(wait_for_rx_with_timeout() < 0) {
            continue;
        }
        radio_read_rx_fifo(rx_buffer);
        if (stop_response->command == SERVER_CLIENT_STOP && stop_response->client_id == CLIENT_ID && stop_response->successful) {
            while(1);
        }
    }
}