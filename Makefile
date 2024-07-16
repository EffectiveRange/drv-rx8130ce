
VERSION = $(shell grep Version: mrhat-rx8130/DEBIAN/control | cut -d' ' -f2)
# TODO: build module for all kernel versions
KVER ?= 6.1.21+
TARGET ?=  $(error TARGET not specified for deploy )

all: build/mrhat-rx8130_$(VERSION)-1_armhf.deb
	@true

build/mrhat-rx8130_$(VERSION)-1_armhf.deb :  build/rtc-rx8130.ko build/mrhat-rx8130.dtbo mrhat-rx8130/DEBIAN/*
	mkdir -p build
	mkdir -p mrhat-rx8130/lib/modules/$(KVER)
	mkdir -p mrhat-rx8130/boot/overlays/
	cp build/rtc-rx8130.ko mrhat-rx8130/lib/modules/$(KVER)/
	cp build/mrhat-rx8130.dtbo mrhat-rx8130/boot/overlays/
	dpkg-deb --root-owner-group --build mrhat-rx8130 build/mrhat-rx8130_$(VERSION)-1_armhf.deb

build/rtc-rx8130.ko: driver/*.c
	mkdir -p build
	rsync --delete -r  ./driver/ /tmp/drv-rx8130ce
	schroot -c buildroot -u root -d /tmp/drv-rx8130ce -- make KVER=$(KVER)
	cp /tmp/drv-rx8130ce/rtc-rx8130.ko build/rtc-rx8130.ko


clean:
	rm -rf mrhat-rx8130/boot/ mrhat-rx8130/lib/ build/

build/mrhat-rx8130.dts.pre: mrhat-rx8130.dts
	mkdir -p build/
	cpp -nostdinc -undef -x assembler-with-cpp -I/var/chroot/buildroot/usr/src/linux-headers-$(KVER)/include -o build/mrhat-rx8130.dts.pre mrhat-rx8130.dts

build/mrhat-rx8130.dtbo: build/mrhat-rx8130.dts.pre
	mkdir -p build/
	dtc  -I dts -O dtb -o build/mrhat-rx8130.dtbo build/mrhat-rx8130.dts.pre

deploy: all
	rsync -avhz --progress build/mrhat-rx8130_$(VERSION)-1_armhf.deb $(TARGET):/tmp/
	ssh $(TARGET) -- sudo dpkg -r mrhat-rx8130
	ssh $(TARGET) -- sudo dpkg -i /tmp/mrhat-rx8130_$(VERSION)-1_armhf.deb

quickdeploy: build/rtc-rx8130.ko
	scp build/rtc-rx8130.ko $(TARGET):/tmp/
	ssh $(TARGET) -- sudo cp /tmp/rtc-rx8130.ko /lib/modules/$(KVER)/
	ssh $(TARGET) -- "sudo rmmod --force rtc-rx8130 || true"
	ssh $(TARGET) -- "sudo modprobe rtc-rx8130"
	

.PHONY: clean all deploy quickdeploy
