#ifndef _DATE_TIME_H /* Guard against multiple inclusion */
#define _DATE_TIME_H

#define YEAR_POS 5
#define MONTH_POS 10
#define DAY_POS 13

#define DATE_CURSOR_YEAR "         ^      "
#define DATE_CURSOR_MONTH "            ^   "
#define DATE_CURSOR_DAY "               ^"

typedef struct
{
    unsigned int year;
    unsigned int month;
    unsigned int day;
    unsigned int hour;
    unsigned int minute;
    unsigned int second;
} date_time_t;

void select_date_time(date_time_t *date_time);
void update_date(date_time_t *date_time);

#endif /* _DATE_TIME_H */
