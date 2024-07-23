cmake_minimum_required(VERSION  3.0)

include(GNUInstallDirs)

project(rtctest)

add_executable(rtc-ioctl  ioctl.c)

add_executable(rtctest  rtctest.c)

install_targets(/bin rtc-ioctl rtctest)