cmake_minimum_required(VERSION  3.0)

include(GNUInstallDirs)

project(rtctest)

add_executable(rtc-ioctl  ioctl.c)

add_executable(rtctest  rtctest.c)

target_include_directories(rtctest PRIVATE ../driver)

install_targets(/bin rtc-ioctl rtctest)