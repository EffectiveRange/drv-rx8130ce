cmake_minimum_required(VERSION  3.0)

include(GNUInstallDirs)

project(rtctest)

add_executable(rtc-ioctl  ioctl.c rtctime.c)

add_executable(rtctest  rtctest.c rtctime.c)

target_include_directories(rtctest PRIVATE ../driver)
target_include_directories(rtc-ioctl PRIVATE ../driver)

install_targets(/bin rtc-ioctl rtctest)