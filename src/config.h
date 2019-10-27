#include <clock.h>
#include <spi.h>
#include <radio.h>
#include <uart.h>
#include <eeprom.h>
#include <sha-256.h>
#include <timer.h>

#define CLIENT_ID 1

#define CLIENT_SERVER_HELLO 0x00
#define SERVER_CLIENT_ENTROPY 0x01
#define CLIENT_SERVER_UNLOCK 0x02
#define SERVER_CLIENT_STOP 0x03

typedef struct {
    int32_t entropy;
    u8 key[8];
} Key;

Key key = {
        0x00,
        {0xb1, 0x1b, 0x74, 0xcc, 0x63, 0x17, 0xa4, 0xb7}
};

typedef struct {
    u8 command;
    u8 client_id;
    u8 padding[36];
} ClientServerHello;

typedef struct {
    u8 command;
    u8 client_id;
    uint32_t entropy;
    u8 padding[32];
} ServerClientEntropy;

typedef struct {
    u8 command;
    u8 client_id;
    uint32_t entropy;
    u8 hash[32];
} ClientServerUnlock;

typedef struct {
    u8 command;
    u8 client_id;
    u8 successful;
    u8 padding[35];
} ServerClientStop;