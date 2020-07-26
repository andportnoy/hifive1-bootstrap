CC=riscv64-unknown-elf-gcc
CFLAGS=-march=rv32imac -mabi=ilp32 -Os
AS=riscv64-unknown-elf-as
OBJCOPY=riscv64-unknown-elf-objcopy

all: hello.hex

upload: hello.hex
	FULLPATH=$(realpath $<); \
	echo "loadfile $${FULLPATH}\nrnh\nexit" \
	  | JLinkExe -device FE310 -if JTAG -speed 4000 -jtagconf -1,-1 -autoconnect 1

%.hex: %.elf
	$(OBJCOPY) -O ihex $< $@

%.elf: %.S
	$(AS) $< -o $@

%.S: %.c
	$(CC) $(CFLAGS) -S $< -o $@

.PHONY: clean
clean:
	FILES=hehe; \
	echo $$FILES; \
	rm -rf $(wildcard *.hex) $(wildcard *.elf)
