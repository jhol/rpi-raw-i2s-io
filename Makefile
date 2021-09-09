obj-m += \
	rpi-raw-i2s-codec.o \
	rpi-simple-soundcard-mod.o

all: $(obj-m:.o=.ko) rpi-raw-i2s-io.dtbo

$(obj-m:.o=.ko): $(obj-m:.o=.c)
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

install: rpi-raw-i2s-io.dtbo
	cp $(obj-m:.o=.ko) /lib/modules/$(shell uname -r)/
	depmod -a
	cp rpi-raw-i2s-io.dtbo /boot/overlays

%.dtbo: %.dts
	dtc -@ -I dts -O dtb -o $@ $<
