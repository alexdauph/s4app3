#include <xc.h>
#include <sys/attribs.h>
#include "config.h"
#include "date_time.h"

void select_date_time(date_time_t *date_time)
{
    date_time->year = 2022;
    date_time->month = 6;
    date_time->day = 5;
    date_time->hour = 16;
    date_time->minute = 20;
    date_time->second = 0;
    
    select_date(date_time);
    LCD_DisplayClear();
    select_time(date_time);
    LCD_DisplayClear();
}

void select_date(date_time_t *date_time)
{
    int pos = 0;
    int days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

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
        }
    }
    while (BTN_GetValue('C') != 0)
        ;
}

void select_time(date_time_t *date_time)
{
    int pos = 0;

    update_time(date_time);
    LCD_WriteStringAtPos(DATE_CURSOR_HOUR, 1, 0);

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
                LCD_WriteStringAtPos(DATE_CURSOR_MINUTE, 1, 0);
                break;
            case 1:
                pos = 2;
                LCD_WriteStringAtPos(DATE_CURSOR_SECOND, 1, 0);
                break;
            case 2:
                pos = 0;
                LCD_WriteStringAtPos(DATE_CURSOR_HOUR, 1, 0);
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
                LCD_WriteStringAtPos(DATE_CURSOR_SECOND, 1, 0);
                break;
            case 1:
                pos = 0;
                LCD_WriteStringAtPos(DATE_CURSOR_HOUR, 1, 0);
                break;
            case 2:
                pos = 1;
                LCD_WriteStringAtPos(DATE_CURSOR_MINUTE, 1, 0);
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
                if (date_time->hour == 23)
                    date_time->hour = 0;
                else
                    date_time->hour++;
                break;
            case 1:
                if (date_time->minute == 59)
                    date_time->minute = 0;
                else
                    date_time->minute++;
                break;
            case 2:
                if (date_time->second == 59)
                    date_time->second = 0;
                else
                    date_time->second++;
                break;
            default:
                break;
            }

            update_time(date_time);
        }

        if (BTN_GetValue('D') == 1)
        {
            while (BTN_GetValue('D') == 1)
                ;
            switch (pos)
            {
            case 0:
                if (date_time->hour == 0)
                    date_time->hour = 23;
                else
                    date_time->hour--;
                break;
            case 1:
                if (date_time->minute == 0)
                    date_time->minute = 59;
                else
                    date_time->minute--;
                break;
            case 2:
                if (date_time->second == 0)
                    date_time->second = 59;
                else
                    date_time->second--;
                break;
            default:
                break;
            }

            update_time(date_time);
        }
    }
    while (BTN_GetValue('C') != 0)
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

void update_time(date_time_t *date_time)
{
    LCD_WriteStringAtPos("Time  ", 0, 0);
    LCD_WriteIntAtPos(date_time->hour, 3, 0, HOUR_POS, 0);
    LCD_WriteIntAtPos(date_time->minute, 3, 0, MINUTE_POS, 0);
    LCD_WriteIntAtPos(date_time->second, 3, 0, SECOND_POS, 0);
    LCD_WriteStringAtPos(":", 0, 8);
    LCD_WriteStringAtPos(":", 0, 11);
}

void date_time_1s(date_time_t *date_time)
{
    int days_in_month[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    date_time->second++;

    if(date_time->second == 60)
    {
        date_time->second = 0;
        date_time->minute++;
    }

    if(date_time->minute == 60)
    {
        date_time->minute = 0;
        date_time->hour++;
    }

    if(date_time->hour == 24)
    {
        date_time->hour = 0;
        date_time->day++;
    }

    if(date_time->day ==days_in_month[date_time->month])
    {
        date_time->day = 1;
        date_time->month++;
    }

    if(date_time->month == 13)
    {
        date_time->month = 1;
        date_time->year++;
    }

    if(date_time->year == 2031)
    {
        date_time->year = 1970;
    }
}