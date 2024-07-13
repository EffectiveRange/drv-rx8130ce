
VERSION = $(shell grep Version: mrhat-rx8130/DEBIAN/control | cut -d' ' -f2)

all: mrhat-rx8130/boot/overlays/mrhat-rx8130.dtbo
	mkdir -p build
	dpkg-deb --root-owner-group --build mrhat-rx8130 build/mrhat-rx8130_$(VERSION)-1_all.deb

clean:
	rm -rf mrhat-rx8130/boot/ build/

mrhat-rx8130/boot/overlays/mrhat-rx8130.dtbo: mrhat-rx8130.dts
	mkdir -p mrhat-rx8130/boot/overlays/
	dtc -I dts -O dtb -o mrhat-rx8130/boot/overlays/mrhat-rx8130.dtbo mrhat-rx8130.dts

.PHONY: clean all
