#include <eeprom.h>

int eeprom_write(u8 address, u8 * data, u8 len) {
    // Unlock the Data area.
    FLASH->DUKR = 0xAE;
    FLASH->DUKR = 0x56;

    delay(1);

    // Check the DUL (data unlocked flag) to check the MASS keys were written correctly.
    if (((FLASH->IAPSR >> 3) & 1) == 0) {
        return -1;
    }

    for (u8 a = 0;a < len; a++) {
        *((u8 *)0x4000 + address + a) = data[a];
    }

    // Clear the DUL bit, re-enabling write protection to the data area.
    FLASH->IAPSR &= ~(1 << 3);

    return 0;
}

void eeprom_read(u8 address, u8 * data, u8 len) {
    for (u8 a = 0; a < len; a++) {
        data[a] = *(u8 *)(0x4000 + address + a);
    }
}