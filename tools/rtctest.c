/*
 *      Real Time Clock Driver Test/Example Program
 *
 *      Compile with:
 *		     gcc -s -Wall -Wstrict-prototypes rtctest.c -o rtctest
 *
 *      Copyright (C) 1996, Paul Gortmaker.
 *
 *      Released under the GNU General Public License, version 2,
 *      included herein by reference.
 *
 */

#include <errno.h>
#include <fcntl.h>
#include <linux/rtc.h>
#include <rtc-rx8130.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "rtctime.h"

/*
 * This expects the new RTC class driver framework, working with
 * clocks that will often not be clones of what the PC-AT had.
 * Use the command line to specify another RTC if you need one.
 */
static const char default_rtc[] = "/dev/rtc0";

int main(int argc, char **argv) {
  int i, fd, retval, irqcount = 0;
  unsigned long tmp, data;
  struct rtc_time rtc_tm;
  struct rtc_wkalrm rtc_wk;
  const char *rtc = default_rtc;

  switch (argc) {
  case 2:
    rtc = argv[1];
    /* FALLTHROUGH */
  case 1:
    break;
  default:
    fprintf(stderr, "usage:  rtctest [rtcdev]\n");
    return 1;
  }

  fd = open(rtc, O_RDONLY);

  if (fd == -1) {
    perror(rtc);
    exit(errno);
  }

  fprintf(stderr, "\n\t\t\tRTC Driver Test Example.\n\n");

  /* Turn on update interrupts (one per second) */
  retval = ioctl(fd, RTC_UIE_ON, 0);
  if (retval == -1) {
    if (errno == ENOTTY) {
      fprintf(stderr, "\n...Update IRQs not supported.\n");
      goto test_READ;
    }
    perror("RTC_UIE_ON ioctl");
    exit(errno);
  }

  fprintf(stderr, "Counting 5 update (1/sec) interrupts from reading %s:", rtc);
  fflush(stderr);
  for (i = 1; i < 6; i++) {
    /* This read will block */
    retval = read(fd, &data, sizeof(unsigned long));
    if (retval == -1) {
      perror("read");
      exit(errno);
    }
    fprintf(stderr, " %d", i);
    fflush(stderr);
    irqcount++;
  }

  fprintf(stderr, "\nAgain, from using select(2) on /dev/rtc:");
  fflush(stderr);
  for (i = 1; i < 6; i++) {
    struct timeval tv = {5, 0}; /* 5 second timeout on select */
    fd_set readfds;

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    /* The select will wait until an RTC interrupt happens. */
    retval = select(fd + 1, &readfds, NULL, NULL, &tv);
    if (retval == -1) {
      perror("select");
      exit(errno);
    }
    /* This read won't block unlike the select-less case above. */
    retval = read(fd, &data, sizeof(unsigned long));
    if (retval == -1) {
      perror("read");
      exit(errno);
    }
    fprintf(stderr, " %d", i);
    fflush(stderr);
    irqcount++;
  }

  /* Turn off update interrupts */
  retval = ioctl(fd, RTC_UIE_OFF, 0);
  if (retval == -1) {
    perror("RTC_UIE_OFF ioctl");
    exit(errno);
  }

test_READ:
  /* Read the RTC time/date */
  read_time_and_get_future_time(fd, &rtc_tm, 5);

  retval = ioctl(fd, RTC_ALM_SET, &rtc_tm);
  if (retval == -1) {
    if (errno == ENOTTY) {
      fprintf(stderr, "\n...Alarm IRQs not supported.\n");
      goto test_PIE;
    }
    perror("RTC_ALM_SET ioctl");
    exit(errno);
  }

  /* Read the current alarm settings */
  retval = ioctl(fd, RTC_ALM_READ, &rtc_tm);
  if (retval == -1) {
    perror("RTC_ALM_READ ioctl");
    exit(errno);
  }

  fprintf(stderr, "Alarm time now set to %02d:%02d:%02d.\n", rtc_tm.tm_hour,
          rtc_tm.tm_min, rtc_tm.tm_sec);

  /* Enable alarm interrupts */
  retval = ioctl(fd, RTC_AIE_ON, 0);
  if (retval == -1) {
    perror("RTC_AIE_ON ioctl");
    exit(errno);
  }

  fprintf(stderr, "Waiting 5 seconds for alarm...");
  fflush(stderr);
  /* This blocks until the alarm ring causes an interrupt */
  retval = read(fd, &data, sizeof(unsigned long));
  if (retval == -1) {
    perror("read");
    exit(errno);
  }
  irqcount++;
  fprintf(stderr, " okay. Alarm rang.\n");

  /* Disable alarm interrupts */
  retval = ioctl(fd, RTC_AIE_OFF, 0);
  if (retval == -1) {
    perror("RTC_AIE_OFF ioctl");
    exit(errno);
  }

test_PIE:
  /* Read periodic IRQ rate */
  retval = ioctl(fd, RTC_IRQP_READ, &tmp);
  if (retval == -1) {
    /* not all RTCs support periodic IRQs */
    if (errno == ENOTTY) {
      fprintf(stderr, "\nNo periodic IRQ support\n");
      goto test_WAKEUP;
    }
    perror("RTC_IRQP_READ ioctl");
    exit(errno);
  }
  fprintf(stderr, "\nPeriodic IRQ rate is %ldHz.\n", tmp);

  fprintf(stderr, "Counting 20 interrupts at:");
  fflush(stderr);

  /* The frequencies 128Hz, 256Hz, ... 8192Hz are only allowed for root. */
  for (tmp = 2; tmp <= 64; tmp *= 2) {

    retval = ioctl(fd, RTC_IRQP_SET, tmp);
    if (retval == -1) {
      /* not all RTCs can change their periodic IRQ rate */
      if (errno == ENOTTY) {
        fprintf(stderr, "\n...Periodic IRQ rate is fixed\n");
        goto test_WAKEUP;
      }
      perror("RTC_IRQP_SET ioctl");
      exit(errno);
    }

    fprintf(stderr, "\n%ldHz:\t", tmp);
    fflush(stderr);

    /* Enable periodic interrupts */
    retval = ioctl(fd, RTC_PIE_ON, 0);
    if (retval == -1) {
      perror("RTC_PIE_ON ioctl");
      exit(errno);
    }

    for (i = 1; i < 21; i++) {
      /* This blocks */
      retval = read(fd, &data, sizeof(unsigned long));
      if (retval == -1) {
        perror("read");
        exit(errno);
      }
      fprintf(stderr, " %d", i);
      fflush(stderr);
      irqcount++;
    }

    /* Disable periodic interrupts */
    retval = ioctl(fd, RTC_PIE_OFF, 0);
    if (retval == -1) {
      perror("RTC_PIE_OFF ioctl");
      exit(errno);
    }
  }

test_WAKEUP:
  read_time_and_get_future_time(fd, &rtc_wk.time, 5);
  // set wakeup timer
  retval = ioctl(fd, SE_RTC_WKTIMER_SET, &rtc_wk.time);
  if (retval == -1) {
    if (errno == ENOTTY) {
      fprintf(stderr, "\n...Wakeup Timer IRQs not supported.\n");
      goto done;
    }
    perror("SE_RTC_WKTIMER_SET ioctl");
    exit(errno);
  }

  for (i = 0; i < 3; ++i) {
    /* Read the current wakeup timer settings */
    retval = ioctl(fd, SE_RTC_WKTIMER_GET, &rtc_wk);
    if (retval == -1) {
      perror("RTC_ALM_READ ioctl");
      exit(errno);
    }

    fprintf(stderr,
            "Alarm time now set to %02d:%02d:%02d. Enabled: %d. Pending: %d\n",
            rtc_wk.time.tm_hour, rtc_wk.time.tm_min, rtc_wk.time.tm_sec,
            rtc_wk.enabled, rtc_wk.pending);
    fprintf(stderr, "Reading 3 times 5 seconds for wakeup timer...\n");
    fflush(stderr);
    sleep(5);
  }

  retval = ioctl(fd, SE_RTC_WKTIMER_SET, NULL);
  if (retval == -1) {
    perror("SE_RTC_WKTIMER_SET NULL ioctl");
    exit(errno);
  }
  retval = ioctl(fd, SE_RTC_WKTIMER_GET, &rtc_wk);
  if (retval == -1) {
    perror("RTC_ALM_READ after clear ioctl");
    exit(errno);
  }
  fprintf(stderr,
          "Alarm time now set to %02d:%02d:%02d. Enabled: %d. Pending: %d\n",
          rtc_wk.time.tm_hour, rtc_wk.time.tm_min, rtc_wk.time.tm_sec,
          rtc_wk.enabled, rtc_wk.pending);
done:
  fprintf(stderr, "\n\n\t\t\t *** Test complete ***\n");

  close(fd);

  return 0;
}
