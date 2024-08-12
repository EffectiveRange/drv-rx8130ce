#include <linux/rtc.h>
#include <stdio.h>
// #include <uapi/linux/rtc.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "rtc-rx8130.h"
#include "rtctime.h"

// from linux-src/include/uapi/linux/rtc.h
#ifndef RTC_VL_READ
#define RTC_VL_READ _IOR('p', 0x13, int) /* Voltage low detector */
#endif
#define RTC_VL_CLR _IO('p', 0x14) /* Clear voltage low information */

unsigned htoi(const char *psHex) {
  unsigned i = 0;
  const int fNeg = (*psHex == '-');

  if (fNeg)
    psHex++;

  for (;;) {
    unsigned char cDigit = (unsigned char)((*psHex++ | 0x20) - '0');
    if ((cDigit < 10) || ((cDigit -= 39) < 16))
      i = (i << 4) + cDigit;
    else
      break;
  }

  if (fNeg)
    i = 0 - i;

  return i;
}

void usage(void) {
  printf("  Read from register:\n");
  printf("  sudo ./ioctl read <reg_number>\n");
  printf("\n");
  printf("  Write to register:\n");
  printf("  sudo ./ioctl write <reg_number> <reg_value>\n");
  printf("\n");
  printf("  Write wakeup:\n");
  printf("  sudo ./ioctl wakeup <seconds>\n");
  printf("\n");
  printf("  Clear wakeup:\n");
  printf("  sudo ./ioctl wakeup \n");
  printf("\n");
  printf("  Note:\n");
  printf("  reg_number and reg_value are in hex format\n");
  printf("\n\n");
}

int main(int argc, char **argv) {
  int fd, retval;
  const char *rtc = "/dev/rtc0";
  reg_data reg;

  fd = open(rtc, O_RDWR);

  if (fd == -1) {
    perror(rtc);
    exit(errno);
  }

  if (argc == 1) {
    usage();
    return 0;
  }

  if (!strcmp(argv[1], "read") && argc == 3) {
    reg.number = htoi(argv[2]);
    retval = ioctl(fd, SE_RTC_REG_READ, &reg);
    if (retval == -1) {
      perror("SE_RTC_REG_READ ioctl");
      goto err;
    } else
      printf("REG[%02xh]=>%02xh\n", reg.number, reg.value);
    goto done;
  }

  if (!strcmp(argv[1], "write") && argc == 4) {
    reg.number = htoi(argv[2]);
    reg.value = htoi(argv[3]);
    retval = ioctl(fd, SE_RTC_REG_WRITE, &reg);
    if (retval == -1) {
      perror("SE_RTC_REG_WRITE ioctl");
      goto err;
    } else
      printf("REG[%02xh]<=%02xh\n", reg.number, reg.value);
    goto done;
  }

  if (!strcmp(argv[1], "vlr") && argc == 2) {
    int vl;
    retval = ioctl(fd, RTC_VL_READ, &vl);
    if (retval == -1) {
      perror("RTC_VL_READ ioctl");
      goto err;
    } else
      printf("%d\n", vl);
    goto done;
  }

  if (!strcmp(argv[1], "vlc") && argc == 2) {
    retval = ioctl(fd, RTC_VL_CLR, 0);
    if (retval == -1) {
      perror("RTC_VL_CLR ioctl");
      goto err;
    }

    goto done;
  }

  if (!strcmp(argv[1], "wakeup") && argc == 3) {
    struct rtc_time tm;
    read_time_and_get_future_time(fd, &tm, atoi(argv[2]));
    // set wakeup timer
    retval = ioctl(fd, SE_RTC_WKTIMER_SET, &tm);
    if (retval == -1) {
      perror("SE_RTC_WKTIMER_SET ioctl");
      goto err;
    }

    goto done;
  }
  if (!strcmp(argv[1], "wakeup")) {
    // clear wakeup timer
    retval = ioctl(fd, SE_RTC_WKTIMER_SET, NULL);
    if (retval == -1) {
      perror("SE_RTC_WKTIMER_SET ioctl");
      goto err;
    }

    goto done;
  }

  usage();

done:
  close(fd);

  return 0;
err: {
  int error = errno;
  close(fd);
  exit(error);
}
}
