#include <errno.h>
#include <fcntl.h>
#include <linux/rtc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

void read_time_and_get_future_time(int fd, struct rtc_time *rtc_tm, int secs) {
  int retval = 0;
  /* Read the RTC time/date */
  retval = ioctl(fd, RTC_RD_TIME, rtc_tm);
  if (retval == -1) {
    perror("RTC_RD_TIME ioctl");
    exit(errno);
  }

  fprintf(stderr, "\n\nCurrent RTC date/time is %d-%d-%d, %02d:%02d:%02d.\n",
          rtc_tm->tm_mday, rtc_tm->tm_mon + 1, rtc_tm->tm_year + 1900,
          rtc_tm->tm_hour, rtc_tm->tm_min, rtc_tm->tm_sec);

  /* Set the alarm to 5 sec in the future, and check for rollover */
  rtc_tm->tm_sec += secs;
  if (rtc_tm->tm_sec >= 60) {
    rtc_tm->tm_sec %= 60;
    rtc_tm->tm_min++;
  }
  if (rtc_tm->tm_min == 60) {
    rtc_tm->tm_min = 0;
    rtc_tm->tm_hour++;
  }
  if (rtc_tm->tm_hour == 24) {
    rtc_tm->tm_hour = 0;
    rtc_tm->tm_mday++;
  }
}