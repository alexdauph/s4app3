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
void get_min_max_avg(int *src, int *dest, int len);
void add_data(int *src, int *dest, int *checksum);

extern void pmod_s();
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
    PMODS_InitPin(1, 1, 0, 0, 0); // initialisation du JB1 (RD9))

    int count_1s = 0;
    int count_16s = 0;
    int checksum = 0;
    int i = 0;
    int acl_x[16] = {0};    // X
    int acl_y[16] = {0};    // Y
    int acl_z[16] = {0};    // Z
    int acl_m[16] = {0};    // Module
    int pot_v[16] = {0};    // Potentiometer
    int tx_data[256] = {0}; //
    unsigned init = 0;
    unsigned char swt_old = 0;
    unsigned char swt_cur = 0;
    unsigned int seconde = 0;
    date_time_t date_time = {0};

    macro_enable_interrupts();

    // SPIFLASH_EraseAll();
    select_date_time(&date_time);

    // Main loop
    while (1)
    {
        if (Flag_1s) // Flag d'interruption à chaque 1 ms
        {
            Flag_1s = 0; // Reset flag

            // Do every 1s
            if (count_1s >= 1000 && init == 0)
            {
                count_1s = 0;

                // Read potentiometer data
                pot_v[count_16s] = AIC_Val();

                // Read accelerometer data
                ACL_ReadAll(&acl_x[count_16s], &acl_y[count_16s], &acl_z[count_16s]);
                acl_m[count_16s] = module_s(acl_x[count_16s], acl_y[count_16s], acl_z[count_16s]);

                // Toggle LED
                LED_ToggleValue(0);

                // Display
                swt_cur = SWT_GetValue(0);
                if (swt_cur != swt_old)
                    LCD_DisplayClear();
                swt_old = swt_cur;

                date_time_1s(&date_time);

                if (swt_cur == 0)
                {
                    LCD_WriteIntAtPos(pot_v[count_16s], 5, 0, 11, 0);
                    LCD_WriteStringAtPos("P", 0, 11);
                    LCD_time(&date_time);
                }
                else
                {
                    LCD_acl(acl_x[count_16s], acl_y[count_16s], acl_z[count_16s], acl_m[count_16s]);
                }

                count_16s++;
            }

            // Do every 16s
            if (count_16s >= 16 && init == 0)
            {
                count_16s = 0;

                // Build frame
                checksum = 0;
                tx_data[0] = (0b0101 << 8) | (255); // Oscillator + data length
                tx_data[1] = seconde;               // Timestamp
                checksum ^= (tx_data[0] & 0x00000F00) >> 8;
                checksum ^= (tx_data[0] & 0x000000F0) >> 4;
                checksum ^= (tx_data[0] & 0x0000000F) >> 0;
                checksum ^= (tx_data[1] & 0xF0000000) >> 28UL;
                checksum ^= (tx_data[1] & 0x0F000000) >> 24UL;
                checksum ^= (tx_data[1] & 0x00F00000) >> 20UL;
                checksum ^= (tx_data[1] & 0x000F0000) >> 16UL;
                checksum ^= (tx_data[1] & 0x0000F000) >> 12UL;
                checksum ^= (tx_data[1] & 0x00000F00) >> 8UL;
                checksum ^= (tx_data[1] & 0x000000F0) >> 4UL;
                checksum ^= (tx_data[1] & 0x0000000F) >> 0UL;
                add_data(acl_x, tx_data + X_OFFSET, &checksum);
                add_data(acl_y, tx_data + Y_OFFSET, &checksum);
                add_data(acl_z, tx_data + Z_OFFSET, &checksum);
                add_data(acl_m, tx_data + M_OFFSET, &checksum);
                add_data(pot_v, tx_data + P_OFFSET, &checksum);
                tx_data[C_OFFSET] = checksum;

                // Send frame to UART
                UART_SendFrame(tx_data);
            }

            count_1s++;
        }
    }
}

void LCD_time(date_time_t *date_time)
{
    LCD_WriteIntAtPos(seconde % 60, 3, 0, 6, 0); // seconds
    LCD_WriteStringAtPos(":", 0, 6);
    LCD_WriteIntAtPos(seconde / 60 % 60, 3, 0, 3, 0); // minutes
    LCD_WriteStringAtPos(":", 0, 3);
    LCD_WriteIntAtPos(seconde / 3600 % 24, 3, 0, 0, 0); // hours
    LCD_WriteStringAtPos("H", 0, 0);
}

void LCD_acl(int x, int y, int z, int module)
{
    char buff[16] = {0};

    int_to_hex_string(buff, x, 3);
    LCD_WriteStringAtPos("x=", 0, 0);
    LCD_WriteStringAtPos(buff, 0, 2);

    int_to_hex_string(buff, y, 3);
    LCD_WriteStringAtPos("y=", 0, 11);
    LCD_WriteStringAtPos(buff, 0, 13);

    int_to_hex_string(buff, z, 3);
    LCD_WriteStringAtPos("z=", 1, 0);
    LCD_WriteStringAtPos(buff, 1, 2);

    int_to_hex_string(buff, module, 3);
    LCD_WriteStringAtPos("m=", 1, 11);
    LCD_WriteStringAtPos(buff, 1, 13);
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

void add_data(int *src, int *dest, int *checksum)
{
    int i;
    int stats[3] = {0};

    get_min_max_avg(src, stats, 16);
    for (i = 0; i < 19; i++)
    {
        if (i < 16)
            dest[i] = src[i];
        else
            dest[i] = stats[i - 16];

        /*checksum ^= (dest[i] & 0xF0000000) >> 28UL;
         *checksum ^= (dest[i] & 0x0F000000) >> 24UL;
         *checksum ^= (dest[i] & 0x00F00000) >> 20UL;
         *checksum ^= (dest[i] & 0x000F0000) >> 16UL;
         *checksum ^= (dest[i] & 0x0000F000) >> 12UL;*/
        *checksum ^= (dest[i] & 0x00000F00) >> 8UL;
        *checksum ^= (dest[i] & 0x000000F0) >> 4UL;
        *checksum ^= (dest[i] & 0x0000000F) >> 0UL;
    }
}

void UART_SendFrame(int *src)
{
    char buff[16] = {0};

    int_to_hex_string(buff, (src[0] & 0x00000F00) >> 8, 1);
    UART_PutString("\nOscillation : 0x");
    UART_PutString(buff);

    int_to_hex_string(buff, (src[0] & 0x000000FF) >> 0, 2);
    UART_PutString("\nCount : 0x");
    UART_PutString(buff);

    int_to_hex_string(buff, (src[M_OFFSET + 16] & 0x00000FFF), 3);
    UART_PutString("\nModule min : 0x");
    UART_PutString(buff);

    int_to_hex_string(buff, (src[M_OFFSET + 17] & 0x00000FFF), 3);
    UART_PutString("\nModule max : 0x");
    UART_PutString(buff);

    int_to_hex_string(buff, (src[M_OFFSET + 18] & 0x00000FFF), 3);
    UART_PutString("\nModule avg : 0x");
    UART_PutString(buff);

    int_to_hex_string(buff, (src[C_OFFSET] & 0x0000000F), 1);
    UART_PutString("\nChecksum : 0x");
    UART_PutString(buff);

    /*for (i = 1; i < 96; i++)
    {
        UART_PutString("\n[] : ");
        UART_PutChar((char)((tx_data[i] & 0xFF000000) >> 24));
        UART_PutChar((char)((tx_data[i] & 0x00FF0000) >> 16));
        UART_PutChar((char)((tx_data[i] & 0x0000FF00) >> 8));
        UART_PutChar((char)((tx_data[i] & 0x000000FF) >> 0));
    }
    UART_PutString("\nChecksum : ");
    UART_PutChar((char)((tx_data[C_OFFSET] & 0x000000FF) >> 0)); // Send checksum*/
}
