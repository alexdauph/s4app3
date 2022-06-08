/************************************
 * Name :     main.c
 * Author:    Jp Gouin
 * Version :  h2022
 ************************************/

#include <xc.h>
#include <sys/attribs.h>
#include "config.h"
#include "date_time.h"
#include <string.h>

#define TMR_TIME 0.001 // x us for each tick
#define X_OFFSET 2
#define Y_OFFSET 21
#define Z_OFFSET 40
#define M_OFFSET 59
#define P_OFFSET 78
#define C_OFFSET 97

// Since the flag is changed within an interrupt, we need the keyword volatile.
static volatile int Flag_1s = 0;

void LCD_time(date_time_t *date_time);
void LCD_acl(int x, int y, int z, int module);
void UART_SendFrame(int *src);
void ACL_ReadAll(int *x, int *y, int *z);

void int_to_hex_string(char *buff, int val, int len);
void int_to_string(char *buff, int val);
void get_min_max_avg(int *src, int *dest, int len);
void add_bloc(int *src, int *dest, int *checksum);
void insert_data(int src, int *dest, int *checksum);

extern void pmod_s(int *data, int *new);
extern long module_s(int x, int y, int z);

void __ISR(_TIMER_1_VECTOR, IPL2AUTO) Timer1ISR(void)
{
    Flag_1s = 1;       //    Indique à la boucle principale qu'on doit traiter
    IFS0bits.T1IF = 0; //    clear interrupt flag
}

void initialize_timer_interrupt(void)
{
    T1CONbits.TCKPS = 3;                                   //    256 prescaler value
    T1CONbits.TGATE = 0;                                   //    not gated input (the default)
    T1CONbits.TCS = 0;                                     //    PCBLK input (the default)
    PR1 = (int)(((float)(TMR_TIME * PB_FRQ) / 256) + 0.5); // set period register, generates one interrupt every 1 ms
                                                           //     48 kHz * 1 ms / 256 = 188
    TMR1 = 0;                                              //    initialize count to 0
    IPC1bits.T1IP = 2;                                     //    INT step 4: priority
    IPC1bits.T1IS = 0;                                     //    subpriority
    IFS0bits.T1IF = 0;                                     //    clear interrupt flag
    IEC0bits.T1IE = 1;                                     //    enable interrupt
    T1CONbits.ON = 1;                                      //    turn on Timer5
}

void main()
{
    LCD_Init();
    LED_Init();
    ACL_Init();
    AIC_Init();
    SWT_Init();
    BTN_Init();
    UART_Init(9600);
    SPIFLASH_Init();

    initialize_timer_interrupt();

    PMODS_InitPin(0, 1, 0, 0, 0); // initialisation du JB1 (RC2)) pour D0
    PMODS_InitPin(0, 2, 0, 0, 0); // initialisation du JA2 (RC1)) pour D1
    PMODS_InitPin(0, 3, 0, 0, 0); // initialisation du JA3 (RC4)) pour D2
    PMODS_InitPin(0, 4, 0, 0, 0); // initialisation du JA4 (RG6)) pour D3
    PMODS_InitPin(0, 7, 0, 0, 0); // initialisation du JA4 (RC3)) pour parite
    PMODS_InitPin(0, 8, 1, 0, 0); // initialisation du JA4 (RC7)) pour ack

    int count_1s = 0;
    int count_16s = 0;
    int checksum = 0;
    int pmod_new = 0;
    int i = 0;
    int buff[5] = {0};      //
    int acl_x[16] = {0};    // X
    int acl_y[16] = {0};    // Y
    int acl_z[16] = {0};    // Z
    int acl_m[16] = {0};    // Module
    int pot_v[16] = {0};    // Potentiometer
    int tx_data[256] = {0}; //
    unsigned char init = 0;
    unsigned char ack = 0;
    unsigned char swt_old = 0;
    unsigned char swt_cur = 0;
    unsigned char waiting = 0;
    unsigned int seconds = 0;
    date_time_t date_time = {0};

    macro_enable_interrupts();

    SPIFLASH_EraseAll(); // TODO uncomment
    // select_date_time(&date_time);        // TODO uncomment

    // Main loop
    while (1)
    {

        pmod_s(tx_data, &pmod_new);
        // if(pmod_new == 1)
        // pmod_new = 0;

        if (Flag_1s) // Flag d'interruption à chaque 1 ms
        {
            Flag_1s = 0; // Reset flag

            // Faire osciller la ligne ACK pour continuer à envoyer du data
            PMODS_SetValue(7, 8, ack);
            ack = !ack;

            // Do every 1s
            if (count_1s >= 1000 && init == 0)
            {
                count_1s = 0;

                // Read potentiometer data
                pot_v[count_16s] = AIC_Val();

                // Read accelerometer data
                ACL_ReadAll(&acl_x[count_16s], &acl_y[count_16s], &acl_z[count_16s]);
                acl_m[count_16s] = module_s(acl_x[count_16s], acl_y[count_16s], acl_z[count_16s]);

                // Send to flash
                buff[0] = acl_x[count_16s];
                buff[1] = acl_y[count_16s];
                buff[2] = acl_z[count_16s];
                buff[3] = acl_m[count_16s];
                buff[4] = pot_v[count_16s];
                SPIFLASH_WriteValues(count_16s, buff);

                // Toggle LED
                LED_ToggleValue(0);

                // Display
                swt_cur = SWT_GetValue(0);
                if (swt_cur != swt_old)
                    LCD_DisplayClear();
                swt_old = swt_cur;

                if (swt_cur == 0)
                {
                    LCD_WriteIntAtPos(pot_v[count_16s], 5, 0, 11, 0);
                    LCD_WriteStringAtPos("P", 0, 11);
                    LCD_time(&date_time);
                    if (waiting == 1)
                        LCD_WriteStringAtPos("Waiting for ACK", 1, 0);
                    else
                        LCD_WriteStringAtPos("                ", 1, 0);
                }
                else
                {
                    LCD_acl(acl_x[count_16s], acl_y[count_16s], acl_z[count_16s], acl_m[count_16s]);
                }

                // Increment date/time and second counter
                date_time_1s(&date_time);
                seconds++;
                count_16s++;
            }

            // Do every 16s
            if (count_16s >= 16 && init == 0)
            {
                count_16s = 0;

                // Retrieve data from flash
                /*for (i = 0; i < 16; i++)
                {
                    SPIFLASH_ReadValues(count_16s, buff);
                    acl_x[X_OFFSET + i] = buff[0];
                    acl_y[Y_OFFSET + i] = buff[1];
                    acl_z[Z_OFFSET + i] = buff[2];
                    acl_m[M_OFFSET + i] = buff[3];
                    pot_v[P_OFFSET + i] = buff[4];
                }*/

                // Build frame
                checksum = 0;
                insert_data((0b0101 << 8) | (95), &tx_data[0], &checksum); // Oscillator + data length
                insert_data(seconds, &tx_data[1], &checksum);              // Timestamp
                add_bloc(acl_x, tx_data + X_OFFSET, &checksum);
                add_bloc(acl_y, tx_data + Y_OFFSET, &checksum);
                add_bloc(acl_z, tx_data + Z_OFFSET, &checksum);
                add_bloc(acl_m, tx_data + M_OFFSET, &checksum);
                add_bloc(pot_v, tx_data + P_OFFSET, &checksum);
                tx_data[C_OFFSET] = checksum;

                // Send frame to UART
                UART_SendFrame(tx_data);

                // Set PMOD to send from frame
                pmod_new = 1;

                // Erase flash
                // TODO
            }

            count_1s++;
        }
    }
}

void LCD_time(date_time_t *date_time)
{
    LCD_WriteIntAtPos(date_time->second, 3, 0, 6, 0); // seconds
    LCD_WriteStringAtPos(":", 0, 6);
    LCD_WriteIntAtPos(date_time->minute, 3, 0, 3, 0); // minutes
    LCD_WriteStringAtPos(":", 0, 3);
    LCD_WriteIntAtPos(date_time->hour, 3, 0, 0, 0); // hours
    LCD_WriteStringAtPos("H", 0, 0);
}

void LCD_acl(int x, int y, int z, int module)
{
    LCD_WriteStringAtPos("x=", 0, 0);
    LCD_WriteIntAtPos(x, 5, 0, 2, 0);

    LCD_WriteStringAtPos("y=", 0, 9);
    LCD_WriteIntAtPos(y, 5, 0, 11, 0);

    LCD_WriteStringAtPos("z=", 1, 0);
    LCD_WriteIntAtPos(z, 5, 1, 2, 0);

    LCD_WriteStringAtPos("m=", 1, 9);
    LCD_WriteIntAtPos(module, 5, 1, 11, 0);
}

void int_to_hex_string(char *buff, int val, int len)
{
    int i;

    for (i = 0; i < len; i++)
    {
        buff[i] = ((val >> (4 * (len - 1 - i))) & 0x0000000F);

        if (buff[i] < 10)
            buff[i] += '0';
        else
            buff[i] += ('A' - 10);
    }
    buff[i] = '\0';
}

void int_to_string(char *buff, int val)
{
    int i;

    if (val < 0)
    {
        buff[0] = '-';
        val *= -1;
    }
    else
        buff[0] = ' ';

    buff[1] = '0' + (val % 10000) / 1000;
    buff[2] = '0' + (val % 1000) / 100;
    buff[3] = '0' + (val % 100) / 10;
    buff[4] = '0' + (val % 10) / 1;
}

void ACL_ReadAll(int *x, int *y, int *z)
{
    char buff[6];

    ACL_ReadRawValues(buff);

    *x = ((buff[0] & 0xFF) << 4) | ((buff[1] & 0xF0) >> 4);
    *y = ((buff[2] & 0xFF) << 4) | ((buff[3] & 0xF0) >> 4);
    *z = ((buff[4] & 0xFF) << 4) | ((buff[5] & 0xF0) >> 4);

    if (*x & 0x0800)
        *x |= 0xFFFFF000;
    if (*y & 0x0800)
        *y |= 0xFFFFF000;
    if (*z & 0x0800)
        *z |= 0xFFFFF000;
}

void get_min_max_avg(int *src, int *dest, int len)
{
    int i;
    int min = src[0];
    int max = src[0];
    int avg = 0;

    for (i = 0; i < len; i++)
    {
        if (src[i] < min)
            min = src[i];
        if (src[i] > max)
            max = src[i];
        avg += src[i];
    }
    avg /= len;

    dest[0] = min;
    dest[1] = max;
    dest[2] = avg;
}

void add_bloc(int *src, int *dest, int *checksum)
{
    int i;
    int stats[3] = {0};

    get_min_max_avg(src, stats, 16);
    for (i = 0; i < 19; i++)
    {
        if (i < 16)
            insert_data(src[i], &dest[i], checksum);
        else
            insert_data(stats[i - 16], &dest[i], checksum);
    }
}

void insert_data(int data, int *dest, int *checksum)
{
    *dest = data;

    *checksum ^= (*checksum & 0xF0000000) >> 28UL;
    *checksum ^= (*checksum & 0x0F000000) >> 24UL;
    *checksum ^= (*checksum & 0x00F00000) >> 20UL;
    *checksum ^= (*checksum & 0x000F0000) >> 16UL;
    *checksum ^= (*checksum & 0x0000F000) >> 12UL;
    *checksum ^= (*checksum & 0x00000F00) >> 8UL;
    *checksum ^= (*checksum & 0x000000F0) >> 4UL;
    *checksum ^= (*checksum & 0x0000000F) >> 0UL;
}

void UART_SendFrame(int *src)
{
    char buff[16] = {0};
    unsigned char* txt1[5] = {"\nX ", "\nY ", "\nZ ", "\nMod ", "\nPot "};
    unsigned char* txt2[3] = {"min : ", "max : ", "avg : "};
    int i, j;
    int offsets[5] = {X_OFFSET, Y_OFFSET, Z_OFFSET, M_OFFSET, P_OFFSET};
    
    UART_PutString("\n\n-----------------------");
    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < 3; j++)
        {
            int_to_string(buff, src[offsets[i] + 16 + j]);
            UART_PutString(txt1[i]);
            UART_PutString(txt2[j]);
            UART_PutString(buff);
        }
        UART_PutString("\n");
    }
    UART_PutString("-----------------------");
}
