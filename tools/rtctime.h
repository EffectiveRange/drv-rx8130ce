#pragma once

#include <linux/rtc.h>

void read_time_and_get_future_time(int fd, struct rtc_time *rtc_tm, int secs);