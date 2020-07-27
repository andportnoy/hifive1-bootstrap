CC=riscv64-unknown-elf-gcc
AS=riscv64-unknown-elf-as
LD=riscv64-unknown-elf-ld
OBJCOPY=riscv64-unknown-elf-objcopy

CFLAGS=-march=rv32imac -mabi=ilp32 -Os
LDFLAGS=-m elf32lriscv -Ttext 0x20010000 -e 0x20010000
ASFLAGS=-march=rv32imac -mabi=ilp32

HEX ?= hello.hex

all: $(HEX)

upload: $(HEX)
	FULLPATH=$(realpath $<); \
	echo "loadfile $${FULLPATH}\nrnh\nexit" \
	  | JLinkExe -device FE310 -if JTAG -speed 4000 -jtagconf -1,-1 -autoconnect 1

%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

%.elf: %.o
	$(LD) $(LDFLAGS) $< -o $@

%.o: %.S
	$(AS) $(ASFLAGS) $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

%.S: %.c
	$(CC) $(CFLAGS) -S $< -o $@

.PHONY: clean
clean:
	rm -rf $(wildcard *.hex) $(wildcard *.elf) $(wildcard *.S) $(wildcard *.o)
