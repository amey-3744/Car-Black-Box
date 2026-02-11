/*
 * File:   car_black_box.c
 * Author: Amey Gunde
 *
 * Created on 7 March, 2025, 10:25 PM
 */

#include "main.h"

extern unsigned int sec;
unsigned char clock_reg[3]; /* HH MM SS */
unsigned char time[7];
unsigned char log[11];
unsigned char return_time = 5;
char *menu[] = {"View log", "Clear log", "Download log", "Change passwd", "Set time"};
int pos = -1;
int event_count = -1;


static void get_time()
{
    clock_reg[0] = read_ds1307(HOUR_ADDR); // HH -> BCD 

    clock_reg[1] = read_ds1307(MIN_ADDR); // MM -> BCD 

    clock_reg[2] = read_ds1307(SEC_ADDR); // SS -> BCD 

    time[0] = ((clock_reg[0] >> 4) & 0x03) + '0';
    time[1] = (clock_reg[0] & 0x0F) + '0';

    time[2] = ((clock_reg[1] >> 4) & 0x07) + '0';
    time[3] = (clock_reg[1] & 0x0F) + '0';

    time[4] = ((clock_reg[2] >> 4) & 0x07) + '0';
    time[5] = (clock_reg[2] & 0x0F) + '0';
    time[6] = '\0';
}

void display_time(void)
{
    get_time(); 

    /* To display the time in HH format */
    clcd_putch(time[0], LINE2(2));
    clcd_putch(time[1], LINE2(3));
    clcd_putch(':', LINE2(4));

    /* To display the time in MM format */
    clcd_putch(time[2], LINE2(5));
    clcd_putch(time[3], LINE2(6));
    clcd_putch(':', LINE2(7));

    /* To display  the time in SS format */
    clcd_putch(time[4], LINE2(8));
    clcd_putch(time[5], LINE2(9));
}

void display_dashboard(unsigned char event[], unsigned char speed)
{
    clcd_print("  TIME     E  SP", LINE1(0));

    // display time
    display_time();

    // display event
    clcd_print(event, LINE2(11));

    // display speed
    clcd_putch(speed / 10 + '0', LINE2(14));
    clcd_putch(speed % 10 + '0', LINE2(15));
}

void log_event(void)
{
    char addr = 0x05;
    pos++; /* 0 */
    if (pos == 10)
    {
        pos = 0;
    }
    addr = pos * 10 + 5; 

    ext_eeprom_str_write(addr, log);

    if (event_count < 9)
    {
        event_count++;
    }
}

void log_car_event(char event[], unsigned char speed)
{
    get_time();

    strncpy(log, time, 6); // HHMMSS
    strncpy(&log[6], event, 2); // EV

    log[8] = (speed / 10) + '0'; // 1st digit of speed
    log[9] = (speed % 10) + '0'; // 1st digit of speed
    log[10] = '\0';

    log_event(); 
}

void clear_screen(void)
{
    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
    __delay_us(500);
}

unsigned char login(unsigned char key, unsigned char reset_flag)
{
    static char npassword[4];
    char spassword[4];
    static char i;
    static unsigned char attempts_rem = 3;

    if (reset_flag == RESET_PASSWORD)
    {
        attempts_rem = 3; 

        npassword[0] = '\0';
        npassword[1] = '\0';
        npassword[2] = '\0';
        npassword[3] = '\0';

        i = 0;
        key = ALL_RELEASED; 
        return_time = 5;
    }

    if (return_time == 0)
    {
        return RETURN_BACK; // return back to dashboard
    }

    __delay_ms(50);

    if (key == SW4 && i < 4)
    {
        npassword[i] = '1';
        clcd_putch('*', LINE2(6 + i));
        i++;
        return_time = 5;
    }
    else if (key == SW5 && i < 4)
    {
        npassword[i] = '0';
        clcd_putch('*', LINE2(6 + i));
        i++;
        return_time = 5;
    }

    if (i == 4)
    {
        for (int j = 0; j < 4; j++)
        {
            spassword[j] = ext_eeprom_read(j);
        }

        if (strncmp(spassword, npassword, 4) == 0)
        {
            // clear the screen and display login success
            clear_screen();
            
            clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
            __delay_us(100);
            
            // login success
            clcd_print("Login Success", LINE1(1));
            __delay_ms(3000);
            
            return LOGIN_SUCCESS;
        }
        else
        {
            attempts_rem--;

            if (attempts_rem == 0) 
            {
                clear_screen();
                
                clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                __delay_us(100);
                
                clcd_print("You are blocked", LINE1(0));
                clcd_print("Wait for", LINE2(0));
                clcd_print("secs ", LINE2(12));

                // wait for 60 sec
                sec = 60;
                
                while (sec)
                {
                    clcd_putch(sec / 10 + '0', LINE2(9));
                    clcd_putch(sec % 10 + '0', LINE2(10));
                }
                attempts_rem = 3; 
            }
            else
            {
                clear_screen();
                
                clcd_print("Wrong password ", LINE1(0));
                clcd_putch(attempts_rem + '0', LINE2(0));
                clcd_print("attempt left  ", LINE2(2));
                __delay_ms(4000);
                
            }

            clear_screen();
            
            clcd_print(" Enter Password ", LINE1(0));
            
            i = 0;
            return_time = 5;
            
            clcd_write(CURSOR_POS, INST_MODE);
            
            clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
            __delay_us(100);
        }
    }
    return 0xFF;
}

unsigned char login_menu(unsigned char key, unsigned char reset_flag)
{
    static char menu_pos;

    if (return_time == 0)
    {
        return RETURN_BACK;
    }

    if (reset_flag == RESET_MENU)
    {
        return_time = 15;
        menu_pos = 0;
        clear_screen();
    }

    if (key == SW4 && menu_pos > 0)
    {
        return_time = 15;
        menu_pos--;
        clear_screen();
    }
    else if (key == SW5 && menu_pos <= 3)
    {
        return_time = 15;
        menu_pos++;
        clear_screen();
    }

    if (menu_pos < 4)
    {
        clcd_putch('*', LINE1(0));
        clcd_print(menu[menu_pos], LINE1(2));
        clcd_print(menu[menu_pos + 1], LINE2(2));
    }
    else if (menu_pos == 4)
    {
        clcd_print(menu[menu_pos - 1], LINE1(2));
        clcd_print(menu[menu_pos], LINE2(2));
        clcd_putch('*', LINE2(0));
    }
    return menu_pos;
}

void view_log(unsigned char key, char reset_index)
{
    static int index;
    char log[11];
    log[10] = 0;
    int position;
    unsigned char add;

    if (event_count == -1)
    {
        clcd_print("                ", LINE1(0));
        clcd_print("    No logs     ", LINE2(0));
    }
    else
    {
        if (reset_index == RESET_VIEW_LOG_POS)
        {
            index = 0;
        }

        if (key == SW4 && index < event_count) 
        {
            index++;
            clcd_print("                ", LINE2(0));
        }
        else if (key == SW5 && index > 0)
        {
            index--;
            clcd_print("                ", LINE2(0));
        }

        position = index;

        for (int i = 0; i < 10; i++)
        {
            add = position * 10 + 5 + i; 
            log[i] = ext_eeprom_read(add);
        }

        // displaying index
        clcd_putch(index + '0', LINE2(0));

        // displaying hours value
        clcd_putch(log[0], LINE2(2));
        clcd_putch(log[1], LINE2(3));
        clcd_putch(':', LINE2(4));

        // displaying minutes  value
        clcd_putch(log[2], LINE2(5));
        clcd_putch(log[3], LINE2(6));
        clcd_putch(':', LINE2(7));

        // displaying seconds  value
        clcd_putch(log[4], LINE2(8));
        clcd_putch(log[5], LINE2(9));

        // displaying event character
        clcd_putch(log[6], LINE2(11));
        clcd_putch(log[7], LINE2(12));

        // displaying speed value
        clcd_putch(log[8], LINE2(14));
        clcd_putch(log[9], LINE2(15));

        __delay_us(500);
    }
}

char clear_log(char reset_memory)
{
    clcd_print("Logs cleared", LINE1(2));
    clcd_print("successfully", LINE2(2));

    if (reset_memory == RESET_MEMORY)
    {
        pos = -1;
        event_count = -1;

        return TASK_SUCCESS;
    }
    return TASK_FAILURE;
}

void download_log(void)
{
    int index = -1;
    char log[11];
    log[10] = 0;
    int position = 0;
    unsigned char add;

    if (event_count == -1)
    {
        puts("No logs available");
    }
    else
    {
        puts("Logs :");
        putchar('\n');
        puts("#     Time       Event    Speed");
        putchar('\n');
        putchar('\r');

        while (index < event_count)
        {
            position = index + 1;
            index++;

            for (int i = 0; i < 10; i++)
            {
                add = position * 10 + 5 + i; 
                log[i] = ext_eeprom_read(add); 
            }

            // Printing index
            putchar(index + '0');       
            puts("     ");             

            // Printing time value (HH:MM:SS)
            putchar(log[0]);               
            putchar(log[1]);              
            putchar(':');                 
            putchar(log[2]);              
            putchar(log[3]);              
            putchar(':');                 
            putchar(log[4]);              
            putchar(log[5]);              
            puts("   ");                  

            // Printing event
            putchar(log[6]);              
            putchar(log[7]);              
            puts("        ");             

            // Printing speed value
            putchar(log[8]);              
            putchar(log[9]);              
            putchar('\n');                
            putchar('\r');                
        }

    }
}

char change_password(unsigned char key, char reset_pwd)
{
    static char new_pwd[9];
    static int pwd_pos = 0;
    static char pwd_changed = 0;
    static unsigned char toggle_cursor = 0;
    static unsigned int blink_delay = 0;

    if (blink_delay++ == 5)
    {
        blink_delay = 0;
        toggle_cursor = !toggle_cursor;
    }

    if (reset_pwd == RESET_PASSWORD)
    {
        strncpy(new_pwd, "    ", 4);
        pwd_pos = 0;
        pwd_changed = 0;
        return_time = 5;
    }

    if (!return_time)
        return RETURN_BACK;

    if (pwd_changed)
        return TASK_FAILURE;

    if (pwd_pos < 4)
    {
        clcd_print("Enter new pwd:  ", LINE1(0));

        if (toggle_cursor == 0)
        {
            clcd_putch((unsigned char) 0xFF, LINE2(pwd_pos));
        }
        else
        {
            clcd_putch(' ', LINE2(pwd_pos));
        }
    }
    else if (pwd_pos > 3 && pwd_pos < 8)
    {
        clcd_print("Re-enter new pwd", LINE1(0));

        if (toggle_cursor == 0)
        {
            clcd_putch((unsigned char) 0xFF, LINE2(pwd_pos));
        }
        else
        {
            clcd_putch(' ', LINE2(pwd_pos));
        }
    }

    switch (key)
    {
        case SW5: 
            new_pwd[pwd_pos] = '0';
            clcd_putch('*', LINE2(pwd_pos));
            pwd_pos++;
            return_time = 5;
            if (pwd_pos == 4)
                clcd_print("                 ", LINE2(0));
            break;

        case SW4:
            new_pwd[pwd_pos] = '1';
            clcd_putch('*', LINE2(pwd_pos));
            pwd_pos++;
            return_time = 5;
            if (pwd_pos == 4)
                clcd_print("                 ", LINE2(0));
            break;
    }

    if (pwd_pos == 8)
    {
        if (strncmp(new_pwd, &new_pwd[4], 4) == 0)
        {
            new_pwd[8] = 0;
            ext_eeprom_str_write(0x00, &new_pwd[4]);
            pwd_pos++;
            
            clear_screen();
            
            clcd_print("Password changed", LINE1(0));
            clcd_print("successfully ", LINE2(2));
            
            pwd_changed = 1;
            __delay_ms(1000);
            return TASK_SUCCESS;
        }
        else
        {
            // displaying password change fail
            clear_screen();
            
            clcd_print("Password  change", LINE1(0));
            clcd_print("failed", LINE2(5));
            
            pwd_changed = 1;
            __delay_ms(1000);

            return TASK_SUCCESS;
        }
    }
    return TASK_FAILURE;
}

char change_time(unsigned char key, unsigned char reset_time)
{
    static unsigned int new_time[3];
    static unsigned int blink_pos;
    static unsigned char wait;
    static unsigned char blink;
    static char t_done = 0;
    char buffer;

    if (reset_time == RESET_TIME)
    {
        get_time();

        // Storing values of new time hours
        new_time[0] = (time[0] & 0x0F) * 10 + (time[1] & 0x0F);

        // Storing values of new time minute
        new_time[1] = (time[2] & 0x0F) * 10 + (time[3] & 0x0F);

        // Storing values of new time second
        new_time[2] = (time[4] & 0x0F) * 10 + (time[5] & 0x0F);

        clcd_print("Time (HH:MM:SS)", LINE1(0));

        blink_pos = 2;
        wait = 0;
        blink = 0;
        t_done = 0;
        key = ALL_RELEASED;
    }

    if (t_done)
        return TASK_FAILURE;

    switch (key)
    {
        case SW4:
            new_time[blink_pos]++;
            break;

        case SW5:
            blink_pos = (blink_pos + 1) % 3;
            break;

        case SW4_LP:

            get_time(); 
            
            buffer = ((new_time[0] / 10) << 4) | new_time[0] % 10; 
            clock_reg[0] = (clock_reg[0] & 0xC0) | buffer; 
            write_ds1307(HOUR_ADDR, clock_reg[0]);

            buffer = ((new_time[1] / 10) << 4) | new_time[1] % 10; 
            clock_reg[1] = (clock_reg[1] & 0x80) | buffer; 
            write_ds1307(MIN_ADDR, clock_reg[1]); 

            buffer = ((new_time[2] / 10) << 4) | new_time[2] % 10; 
            clock_reg[2] = (clock_reg[2] & 0x80) | buffer; 
            write_ds1307(SEC_ADDR, clock_reg[2]); 

            clear_screen();
            
            clcd_print("Time changed", LINE1(2));
            clcd_print("Successfully", LINE2(2));

            t_done = 1;
            __delay_ms(1000);
            return TASK_SUCCESS;
    }

    if (new_time[0] > 23)
        new_time[0] = 0;
    if (new_time[1] > 59)
        new_time[1] = 0;
    if (new_time[2] > 59)
        new_time[2] = 0;

    if (wait++ == 1)
    {
        wait = 0;
        blink = !blink;

        if (blink)
        {
            switch (blink_pos)
            {
                case 0:
                    clcd_print("  ", LINE2(0));
                    __delay_ms(160);
                    break;
                case 1:
                    clcd_print("  ", LINE2(3));
                    __delay_ms(160);
                    break;
                case 2:
                    clcd_print("  ", LINE2(6));
                    __delay_ms(160);
                    break;
            }
        }
    }

    // Displaying hours field
    clcd_putch(new_time[0] / 10 + '0', LINE2(0));
    clcd_putch(new_time[0] % 10 + '0', LINE2(1));
    clcd_putch(':', LINE2(2));

    // Displaying mins field
    clcd_putch(new_time[1] / 10 + '0', LINE2(3));
    clcd_putch(new_time[1] % 10 + '0', LINE2(4));
    clcd_putch(':', LINE2(5));

    // Displaying secs field
    clcd_putch(new_time[2] / 10 + '0', LINE2(6));
    clcd_putch(new_time[2] % 10 + '0', LINE2(7));

    return TASK_FAILURE;
}