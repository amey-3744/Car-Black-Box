/* 
 * File:   eeprom.h
 */

#include "main.h"

#ifndef EEPROM_H
#define	EEPROM_H

#define SLAVE_READ_EE               0xA1
#define SLAVE_WRITE_EE              0xA0
//#define _XTAL_FREQ                  20000000

unsigned char ext_eeprom_read(unsigned char memory_loc);
void ext_eeprom_write(unsigned char memory_loc, unsigned char data);
void ext_eeprom_str_write(unsigned char memory_loc, char *data);

#endif	/* EEPROM_H */