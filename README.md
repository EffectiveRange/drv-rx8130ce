# DT Overlay for Epson RX8130CE RTC on RaspberryPi

## Description

This dtoverlay enables the RTC located at the [MrHAT extension boards](https://github.com/EffectiveRange/pcb-mrhat). The RTC will
appear under `/dev/rtc<X>`,and can be interacted with using the standard Linux RTC API.

## Usage

Install the debian package from the releases.That will place the dtoverlay under the /boot/overlays directory, 
post installation you need to configure the device tree by adding `dtoverlay=mrhat-rx8130` to the /boot/config.txt. 
By default, GPIO23 is used as the RTC interrupt pin, and main battery is enabled without charging, but that can be modified, by manually editing the boot config and specifying it there.
Example for using GPIO5 for IRQ, disable main battery, enable external EDLC chargeable capacitor:
```bash
# in /boot/config.txt
dtoverlay=mrhat-rx8130:rtc_irq_pin=5,ext_battery=false,ext_capacitor=true
```

## License

This project is licensed under the [MIT License](LICENSE).
