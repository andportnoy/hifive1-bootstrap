ARCH=rv32i

CC=riscv64-unknown-elf-gcc
AS=riscv64-unknown-elf-as
LD=riscv64-unknown-elf-ld
OBJCOPY=riscv64-unknown-elf-objcopy
OBJDUMP=riscv64-unknown-elf-objdump

INCLUDE=project.h
CFLAGS =-march=$(ARCH) -mabi=ilp32 -Os -include $(INCLUDE)
CFLAGS+=-nostartfiles -nostdlib -ffunction-sections -fdata-sections -flto
CFLAGS+=-Wall -Wextra -pedantic -std=c11
ASFLAGS=-march=$(ARCH) -mabi=ilp32
LDFLAGS=-T link.lds -Wl,--gc-sections

T ?= clock

all: $(T).hex

clock: utils.o raminit.o
ledcycle: utils.o raminit.o
timer: utils.o raminit.o
test: utils.o raminit.o
pwm: utils.o raminit.o
ultrasonic: utils.o raminit.o

upload: $(T).hex
	FULLPATH=$(realpath $<); \
	echo -e "loadfile $${FULLPATH}\nrnh\nexit" \
	  | JLinkExe -device FE310 -if JTAG -speed 4000 -jtagconf -1,-1 -autoconnect 1

%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

%.lst: %.elf
	$(OBJDUMP) -d -t -M no-aliases $< > $@

%.elf: %
	mv $< $@

%.elf: %.s
	$(AS) $(ASFLAGS) $< -o $*.o
	$(LD) -Ttext 0x20010000 -Tdata 0x80000000 -m elf32lriscv $*.o -o $@

.PHONY: clean
clean:
	rm -rf *.hex *.o *.lst *.elf $T
