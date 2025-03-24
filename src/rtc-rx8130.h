/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#pragma once

#include <linux/types.h>

typedef struct {
  __u8 number;
  __u8 value;
} reg_data;

#define SE_RTC_REG_READ _IOWR('p', 0x20, reg_data)
#define SE_RTC_REG_WRITE _IOW('p', 0x21, reg_data)
#define SE_RTC_WKTIMER_SET _IOW('p', 0x40, struct rtc_time *)
#define SE_RTC_WKTIMER_GET _IOR('p', 0x41, struct rtc_wkalrm *)
