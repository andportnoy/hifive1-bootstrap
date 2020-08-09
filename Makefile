ARCH=rv32i

CC=riscv64-unknown-elf-gcc
AS=riscv64-unknown-elf-as
LD=riscv64-unknown-elf-ld
OBJCOPY=riscv64-unknown-elf-objcopy

INCLUDE=project.h
CFLAGS =-march=$(ARCH) -mabi=ilp32 -Os -include $(INCLUDE)
CFLAGS+=-nostartfiles -nostdlib -ffunction-sections -fdata-sections
ASFLAGS=-march=$(ARCH) -mabi=ilp32
LDFLAGS=-Ttext 0x20010000 -Tdata 0x80000000 -e main -Wl,--gc-sections

T ?= ledcycle

all: $(T).hex

ledcycle: utils.o

upload: $(T).hex
	FULLPATH=$(realpath $<); \
	echo "loadfile $${FULLPATH}\nrnh\nexit" \
	  | JLinkExe -device FE310 -if JTAG -speed 4000 -jtagconf -1,-1 -autoconnect 1

%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

%.lst: %.elf
	riscv64-unknown-elf-objdump -d -M no-aliases $< > $@

%.elf: %
	cp $< $@

%.elf: %.s
	$(AS) $(ASFLAGS) $< -o $*.o
	$(LD) -Ttext 0x20010000 -Tdata 0x80000000 -m elf32lriscv $*.o -o $@

.PHONY: clean
clean:
	rm -rf *.hex *.o *.lst *.elf
