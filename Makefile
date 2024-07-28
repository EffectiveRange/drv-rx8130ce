
VERSION = $(shell grep Version: mrhat-rx8130/DEBIAN/control | cut -d' ' -f2)
# TODO: build module for all kernel versions
KVER ?= 6.1.21+
TARGET ?=  $(error TARGET not specified for deploy )
IRQ_PIN ?= 23

all: build/mrhat-rx8130_$(VERSION)-1_armhf.deb
	@true

build/mrhat-rx8130_$(VERSION)-1_armhf.deb :  driver tools mrhat-rx8130/DEBIAN/*  build/mrhat-rx8130.dtbo
	mkdir -p build
	mkdir -p mrhat-rx8130/boot/overlays/
	cp build/mrhat-rx8130.dtbo mrhat-rx8130/boot/overlays/
	dpkg-deb --root-owner-group --build mrhat-rx8130 build/mrhat-rx8130_$(VERSION)-1_armhf.deb

mrhat-rx8130/lib/modules/$(KVER)/rtc-rx8130.ko: driver/*.c driver/Makefile
	mkdir -p build/$(KVER)
	mkdir -p mrhat-rx8130/lib/modules/$(KVER)
	rsync --delete -r  ./driver/ /tmp/drv-rx8130ce
	schroot -c buildroot -u root -d /tmp/drv-rx8130ce -- make KVER=$(KVER)
	cp /tmp/drv-rx8130ce/rtc-rx8130.ko mrhat-rx8130/lib/modules/$(KVER)/rtc-rx8130.ko

driver: mrhat-rx8130/lib/modules/$(KVER)/rtc-rx8130.ko
	@true

clean:
	rm -rf mrhat-rx8130/boot/ mrhat-rx8130/lib/ mrhat-rx8130/usr build/ tools/build 

build/mrhat-rx8130.dts.pre: mrhat-rx8130.dts
	mkdir -p build/
	cpp -nostdinc -undef -x assembler-with-cpp -I/var/chroot/buildroot/usr/src/linux-headers-$(KVER)/include -o build/mrhat-rx8130.dts.pre mrhat-rx8130.dts

build/mrhat-rx8130.dtbo: build/mrhat-rx8130.dts.pre
	mkdir -p build/
	dtc  -I dts -O dtb -o build/mrhat-rx8130.dtbo build/mrhat-rx8130.dts.pre

deploy: all
	rsync -e "ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null" -avhz --progress build/mrhat-rx8130_$(VERSION)-1_armhf.deb $(TARGET):/tmp/
	ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null $(TARGET) -- sudo dpkg -r mrhat-rx8130
	ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null $(TARGET) -- sudo dpkg -i /tmp/mrhat-rx8130_$(VERSION)-1_armhf.deb
	ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null $(TARGET) -- sudo sed -ri '/^\s*dtoverlay=mrhat-rx8130/d' /boot/config.txt
	ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null $(TARGET) -- "echo 'dtoverlay=mrhat-rx8130:rtc_irq_pin=$(IRQ_PIN)' | sudo tee -a /boot/config.txt"

quickdeploy: driver
	scp mrhat-rx8130/lib/modules/$(KVER)/rtc-rx8130.ko $(TARGET):/tmp/
	ssh -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null $(TARGET) -- sudo cp /tmp/rtc-rx8130.ko /lib/modules/$(KVER)/

tools:
	mkdir -p tools/build
	mkdir -p mrhat-rx8130/usr/local/bin
	xcmake -S tools -B tools/build
	cmake --build tools/build
	cmake --install tools/build --prefix mrhat-rx8130/usr/local/

.PHONY: clean all deploy quickdeploy driver tools
