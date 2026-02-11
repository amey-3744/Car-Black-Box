/* 
 * File:   eeprom.c
 */

#include <xc.h>
#include "i2c.h"
#include "eeprom.h"

unsigned char ext_eeprom_read(unsigned char memory_loc)
{
    unsigned char received_data = 0;

    i2c_start();
    i2c_write(SLAVE_WRITE_EE);
    i2c_write(memory_loc);
    i2c_rep_start();
    i2c_write(SLAVE_READ_EE);
    received_data = i2c_read(0);
    i2c_stop();

    return received_data;
}

void ext_eeprom_write(unsigned char memory_loc, unsigned char data)
{
    i2c_start();
    i2c_write(SLAVE_WRITE_EE);
    i2c_write(memory_loc);
    i2c_write(data);
    i2c_stop();

    __delay_ms(3);
}

void ext_eeprom_str_write(unsigned char memory_loc, char *data)
{
    while (*data != 0)
    {
        ext_eeprom_write(memory_loc, *data);
        data++;
        memory_loc++;
    }
}