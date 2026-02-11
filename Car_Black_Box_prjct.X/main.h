/* 
 * File:   main.h
 */

#ifndef MAIN_H
#define	MAIN_H

#include <xc.h>
#include <string.h>
#include "clcd.h"
#include "adc.h"
#include "ds1307.h"
#include "i2c.h"
#include "car_black_box.h"
#include "digital_keypad.h"
#include "eeprom.h"
#include "timers.h"
#include "uart.h"

#define DASHBOARD_SCREEN          0x01 
#define LOGIN_SCREEN              0x02
#define MENU_SCREEN               0x03
#define VIEW_LOG_SCREEN           0x08    
#define CLEAR_LOG_SCREEN          0x09    
#define DOWNLOAD_LOG_SCREEN       0x10    
#define CHANGE_PASSWORD_SCREEN 	  0x11   
#define SET_TIME_SCREEN	 		  0x12    



#define RESET_DOWNLOAD_LOG        0x10    
#define RESET_VIEW_LOG_POS        0x20    
#define RESET_TIME				  0x30    
#define RESET_MEMORY              0x40    
#define RESET_LOGIN_MENU          0x50   
#define RESET_PASSWORD            0x60   
#define RESET_MENU                0x70   
#define RESET_NOTHING             0x80



#define RETURN_BACK               0x0A   
#define LOGIN_SUCCESS             0x0B   
#define TASK_SUCCESS              0x0C    
#define TASK_FAILURE              0x0D   

#endif	/* MAIN_H */