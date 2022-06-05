#ifndef _DATE_TIME_H /* Guard against multiple inclusion */
#define _DATE_TIME_H

#define YEAR_POS 5
#define MONTH_POS 10
#define DAY_POS 13

#define HOUR_POS 5
#define MINUTE_POS 8
#define SECOND_POS 11

#define DATE_CURSOR_YEAR "         ^      "
#define DATE_CURSOR_MONTH "            ^   "
#define DATE_CURSOR_DAY "               ^"

#define DATE_CURSOR_HOUR "       ^        "
#define DATE_CURSOR_MINUTE "          ^     "
#define DATE_CURSOR_SECOND "             ^  "

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
void select_date(date_time_t *date_time);
void select_time(date_time_t *date_time);
void update_date(date_time_t *date_time);
void update_time(date_time_t *date_time);
void date_time_1s(date_time_t *date_time);

#endif /* _DATE_TIME_H */
