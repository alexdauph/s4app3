#include <xc.h>
#include <sys/attribs.h>
#include "config.h"
#include "date_time.h"

void select_date_time(date_time_t *date_time)
{
    int pos = 0;
    int days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    date_time->year = 2022;
    date_time->month = 6;
    date_time->day = 5;
    date_time->hour = 16;
    date_time->minute = 20;
    date_time->second = 0;

    pos = 0;
    update_date(date_time);
    LCD_WriteStringAtPos(DATE_CURSOR_YEAR, 1, 0);

    while (BTN_GetValue('C') != 1)
    {
        if (BTN_GetValue('R') == 1)
        {
            while (BTN_GetValue('R') == 1)
                ;
            switch (pos)
            {
            case 0:
                pos = 1;
                LCD_WriteStringAtPos(DATE_CURSOR_MONTH, 1, 0);
                break;
            case 1:
                pos = 2;
                LCD_WriteStringAtPos(DATE_CURSOR_DAY, 1, 0);
                break;
            case 2:
                pos = 0;
                LCD_WriteStringAtPos(DATE_CURSOR_YEAR, 1, 0);
                break;
            default:
                pos = 0;
                break;
            }
        }

        if (BTN_GetValue('L') == 1)
        {
            while (BTN_GetValue('L') == 1)
                ;
            switch (pos)
            {
            case 0:
                pos = 2;
                LCD_WriteStringAtPos(DATE_CURSOR_DAY, 1, 0);
                break;
            case 1:
                pos = 0;
                LCD_WriteStringAtPos(DATE_CURSOR_YEAR, 1, 0);
                break;
            case 2:
                pos = 1;
                LCD_WriteStringAtPos(DATE_CURSOR_MONTH, 1, 0);
                break;
            default:
                pos = 0;
                break;
            }
        }

        if (BTN_GetValue('U') == 1)
        {
            while (BTN_GetValue('U') == 1)
                ;
            switch (pos)
            {
            case 0:
                if (date_time->year == 2030)
                    date_time->year = 1970;
                else
                    date_time->year++;
                break;
            case 1:
                if (date_time->month == 12)
                    date_time->month = 1;
                else
                    date_time->month++;
                if (date_time->day > days_in_month[date_time->month - 1])
                    date_time->day = days_in_month[date_time->month - 1];
                break;
            case 2:
                if (date_time->day == days_in_month[date_time->month - 1])
                    date_time->day = 1;
                else
                    date_time->day++;
                break;
            default:
                break;
            }

            update_date(date_time);
            LCD_WriteStringAtPos("/", 0, 10);
            LCD_WriteStringAtPos("/", 0, 13);
        }

        if (BTN_GetValue('D') == 1)
        {
            while (BTN_GetValue('D') == 1)
                ;
            switch (pos)
            {
            case 0:
                if (date_time->year == 1970)
                    date_time->year = 2030;
                else
                    date_time->year--;
                break;
            case 1:
                if (date_time->month == 1)
                    date_time->month = 12;
                else
                    date_time->month--;
                if (date_time->day > days_in_month[date_time->month - 1])
                    date_time->day = days_in_month[date_time->month - 1];
                break;
            case 2:
                if (date_time->day == 1)
                    date_time->day = days_in_month[date_time->month - 1];
                else
                    date_time->day--;
                break;
            default:
                break;
            }

            update_date(date_time);
            LCD_WriteStringAtPos("/", 0, 10);
            LCD_WriteStringAtPos("/", 0, 13);
        }
    }
    while (BTN_GetValue('C') != 0)
        ;

    LCD_DisplayClear();

    while (1)
        ;
}

void update_date(date_time_t *date_time)
{
    LCD_WriteStringAtPos("Date  ", 0, 0);
    LCD_WriteIntAtPos(date_time->year, 5, 0, YEAR_POS, 0);
    LCD_WriteIntAtPos(date_time->month, 3, 0, MONTH_POS, 0);
    LCD_WriteIntAtPos(date_time->day, 3, 0, DAY_POS, 0);
    LCD_WriteStringAtPos("/", 0, 10);
    LCD_WriteStringAtPos("/", 0, 13);
}