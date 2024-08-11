obj-m += rtc-rx8130.o 
KVER ?= $(shell uname -r)
PWD := $(CURDIR) 
 
CFLAGS_rtc-rx8130.o := -O2 -Wall -Werror $(RXCFLAGS)

all: 
	make -C /lib/modules/$(KVER)/build M=$(PWD) modules 
 
clean: 
	make -C /lib/modules/$(KVER)/build M=$(PWD) clean