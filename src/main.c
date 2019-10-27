int main() {
#ifdef CLIENT
#define CLIENT_ID 1
    while (1) {
        u8 message[FIELD_LENGTH];
        message[0] = CLIENT_SERVER_HELLO;
        message[1] = CLIENT_ID;
        transmit_data(message);
        while(!is_tx_finished());
        start_rx();
        delay(10);
        if (!is_data_ready()) {
            //printf("No data received\n");
            continue;
        }
        if (read_message(message) < 0) {
            //printf("Error reading response\n");
            continue;
        }
        if (message[1] != CLIENT_ID) {
            //printf("Invalid client id received\n");
            continue;
        }
        if (message[0] != SERVER_CLIENT_ENTROPY) {
            //printf("Invalid message received\n");
            continue;
        }

        int32_t entropy = *(int32_t *)(message + 2);
        printf("Received entropy %ld\n", entropy);

        *PSK = entropy + 1;
        *(message + 2) = entropy;
        calc_sha_256(message + 6, PSK, sizeof(PSK));

        message[0] = CLIENT_SERVER_UNLOCK;
        message[1] = CLIENT_ID;

        transmit_data(message);
        while(!is_tx_finished());
        printf("Wait for confirmation\n");
        start_rx();
        while(!is_data_ready());
        printf("Got message\n");
        read_message(message);
        putchar('0' + message[0]);
        putchar('\n');
        delay(1000);
    }
#endif




#ifdef TEST_TX
    u8 d[FIELD_LENGTH] = {0, 0, 0, 0, 0, 0, 0};

    while(1) {
        transmit_data(d);
        wiggle();
        start_tx();
        wiggle();
        while(!is_tx_finished());
        wiggle();
    }
#endif

}
