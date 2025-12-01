#include "utils.h"

#include <pebble.h>

void get_time_digits(int *digits) {
    time_t now_ = time(NULL);
    struct tm *now = localtime(&now_);
    int hour = now->tm_hour;
    if (!clock_is_24h_style()) {
        hour %= 12;
        if (hour == 0) {
            hour = 12;
        }
    }
    digits[0] = hour / 10;
    digits[1] = hour % 10;
    digits[2] = now->tm_min / 10;
    digits[3] = now->tm_min % 10;
}
