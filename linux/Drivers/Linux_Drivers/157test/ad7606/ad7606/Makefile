KERNELDIR :=/home/f-king/linux/atk-mp1/linux/alientek_linux
CURRENT_PATH := $(shell pwd)
obj-m := ad7606_m.o
ad7606_m-objs:=ad7606.o ad7606_spi.o
build: kernel_modules
kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
