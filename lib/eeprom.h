#ifndef __EEPROM_H
#define __EEPROM_H

#include <stm8s.h>
#include <util.h>

int eeprom_write(u8 address, u8 * data, u8 len);
void eeprom_read(u8 address, u8 * data, u8 len);

#endif