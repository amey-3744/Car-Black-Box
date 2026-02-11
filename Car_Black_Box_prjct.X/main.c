/*
 * File:   main.c
 * Author: Amey Gunde
 *
 * Created on 7 March, 2025, 9:48 PM
 */

#include "main.h"

#pragma config WDTE = OFF

static void init_config(void)
{
     // initialize i2c
    init_i2c(100000); // 100kHz
    
    // initialize the RTC
    init_ds1307();
    
    // initialize the ADC
    init_adc();
    
    // initialize the clcd
    init_clcd();
    
    // initialize the digital keypad
    init_digital_keypad();
    
    // initialize th uart 
    init_uart(9600);
    
    // configure timer 2
    init_timer2();
    
    // Peripheral Interrupt Enable Bit (For Timer2)
    PEIE = 1;
    
    // Enable all the Global Interrupts
    GIE = 1;
}

/* main function */
void main(void)
{
    /* initialization */
    unsigned char control_flag = DASHBOARD_SCREEN, key; 
    unsigned char event[3] = "ON";

    // index          0     1     2     3     4     5
    char *gear[] = {"GR", "GN", "G1", "G2", "G3", "G4"};
    
    unsigned char speed = 0, gr = 0;
    
    unsigned char reset_flag; 
    
    unsigned char menu_pos;

    init_config();

    ext_eeprom_str_write(0x00, "1010");

    log_car_event(event, speed);

    while (1) /*super loop */
    {
        speed = read_adc() / 10; 

        if (speed > 99) 
        {
            speed = 99;
        }

        key = read_digital_keypad(); 

        if (key == SW1)
        {
            strcpy(event, "CO");
            log_car_event(event, speed); 
        }
        else if (key == SW2 && gr < 6) 
        {
            strcpy(event, gear[gr]); 
            gr++;
            log_car_event(event, speed); 
        }
        else if (key == SW3 && gr > 0) 
        {
            gr--; 
            strcpy(event, gear[gr]); 
            log_car_event(event, speed); 
        }
        else if ((key == SW4 || key == SW5) && control_flag == DASHBOARD_SCREEN) 
        {
            clear_screen();

            clcd_print(" Enter Password ", LINE1(0));

            clcd_write(CURSOR_POS, INST_MODE);
            
            clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
            __delay_us(100);

            control_flag = LOGIN_SCREEN; 

            reset_flag = RESET_PASSWORD; 

            TMR2ON = 1;
        }
        else if (key == SW4_LP && control_flag == MENU_SCREEN)
        {
            switch (menu_pos)
            {
                case 0: // View Log

                    clear_screen();
                    clcd_print("      Logs      ", LINE1(0));
                    control_flag = VIEW_LOG_SCREEN;
                    reset_flag = RESET_VIEW_LOG_POS;
                    break;

                case 1: // Clear Log

                    clear_screen();
                    control_flag = CLEAR_LOG_SCREEN;
                    reset_flag = RESET_MEMORY;
                    break;

                case 2: // Download Log

                    clear_screen();
                    log_car_event("DL", speed);
                    clcd_print("       Open      ", LINE1(0));
                    clcd_print("     Tera Term    ", LINE2(0));
                    download_log();
                    __delay_ms(2000);
                    control_flag = MENU_SCREEN;
                    reset_flag = RESET_LOGIN_MENU;
                    break;

                case 3: // Change Password

                    clear_screen();
                    control_flag = CHANGE_PASSWORD_SCREEN;
                    reset_flag = RESET_PASSWORD;
                    TMR2ON = 1;
                    break;

                case 4: // Set Time

                    clear_screen();
                    log_car_event("ST", speed);
                    control_flag = SET_TIME_SCREEN;
                    reset_flag = RESET_TIME;
                    break;
            }
        }
        else if (key == SW4_LP && control_flag == VIEW_LOG_SCREEN)
        {
            control_flag = MENU_SCREEN;
            clear_screen();
        }
        else if (key == SW4_LP && control_flag == CHANGE_PASSWORD_SCREEN)
        {
            control_flag = MENU_SCREEN;
            clear_screen();
        }
        else if (key == SW5_LP && control_flag == MENU_SCREEN)
        {
            control_flag = DASHBOARD_SCREEN;
            clear_screen();
        }

        
        switch (control_flag)
        {
            case DASHBOARD_SCREEN: // dashboard screen

                display_dashboard(event, speed); 
                break;

            case LOGIN_SCREEN: // login screen

                switch (login(key, reset_flag))
                {
                    case RETURN_BACK:

                        control_flag = DASHBOARD_SCREEN;
                        TMR2ON = 0;

                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        break;

                    case LOGIN_SUCCESS:

                        control_flag = MENU_SCREEN; 
                        reset_flag = RESET_MENU;
                        TMR2ON = 1; 
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        continue;
                }
                break;

            case MENU_SCREEN: // login menu screen

                switch (login_menu(key, reset_flag))
                {
                    case RETURN_BACK:
                        clear_screen();
                        control_flag = DASHBOARD_SCREEN;
                        TMR2ON = 0;
                        break;

                    case 0:
                        menu_pos = 0;
                        break;

                    case 1:
                        menu_pos = 1;
                        break;

                    case 2: 
                        menu_pos = 2;
                        break;

                    case 3: 
                        menu_pos = 3;
                        break;

                    case 4: 
                        menu_pos = 4;
                        break;
                }
                break;

            case VIEW_LOG_SCREEN: // View Log

                view_log(key, reset_flag);
                break;

            case CLEAR_LOG_SCREEN: // Clear Log

                if (clear_log(reset_flag) == TASK_SUCCESS)
                    __delay_ms(1000);

                if (reset_flag == RESET_MEMORY)
                    log_car_event("CL", speed);

                control_flag = MENU_SCREEN;
                reset_flag = RESET_LOGIN_MENU;
                clear_screen();
                break;

            case CHANGE_PASSWORD_SCREEN: // Change Password

                switch (change_password(key, reset_flag))
                {
                    case TASK_SUCCESS:
                        __delay_ms(1000);
                        log_car_event("CP", speed);
                        control_flag = MENU_SCREEN; 
                        reset_flag = RESET_LOGIN_MENU;
                        clear_screen();
                        break;

                    case RETURN_BACK:
                        control_flag = DASHBOARD_SCREEN;
                        reset_flag = RESET_LOGIN_MENU;
                        break;
                }
                break;

            case SET_TIME_SCREEN: // Set Time

                if (change_time(key, reset_flag) == TASK_SUCCESS)
                {
                    control_flag = MENU_SCREEN;
                    reset_flag = RESET_LOGIN_MENU;
                    clear_screen();
                    continue;
                }
                break;
        }
        reset_flag = RESET_NOTHING;
    }
    return;
}