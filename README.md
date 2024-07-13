# DT Overlay for Epson RX8130CE RTC on RaspberryPi

## Description

This dtoverlay enables the RTC located at the [MrHAT extension boards](https://github.com/EffectiveRange/pcb-mrhat). The RTC will
appear under `/dev/rtc<X>`,and can be interacted with using the standard Linux RTC API.

## Usage

Install the debian package from the releases.That will place the dtoverlay under the /boot/overlays directory, 
and the post installation script also adds `dtoverlay=mrhat-rx8130` to the /boot/config.txt if not present. 
By default, GPIO5 is used as the RTC interrupt pin, but that can be modified, by manually editing the boot config and specifying it there.
Example:
```bash
# in /boot/config.txt
dtoverlay=mrhat-rx8130,rtc_irq_pin=18
```

## License

This project is licensed under the [MIT License](LICENSE).
