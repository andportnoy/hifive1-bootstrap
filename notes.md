# Goal
My current goal is to work out a flow that allows to write assembly, assemble
that, and upload to the board. I also want to be able to see what is happening
on the board. Can I dump memory? Can I debug/single step?

- compile to assembly using gcc
- assemble using as
- give it an .elf extension
- convert to .hex file using objcopy (why?)
- upload
ASFLAGS="-march=rv32imac -mabi=ilp32 -mcmodel=medlow --specs=nano.specs -Os"
CCASFLAGS="-march=rv32imac -mabi=ilp32 -mcmodel=medlow -I/home/aportnoy/Projects/riscv/freedom-e-sdk/bsp/sifive-hifive1-revb/install/include --specs=nano.specs"
CFLAGS="-march=rv32imac -mabi=ilp32 -mcmodel=medlow -ffunction-sections -fdata-sections -I/home/aportnoy/Projects/riscv/freedom-e-sdk/bsp/sifive-hifive1-revb/install/include --specs=nano.specs -DMTIME_RATE_HZ_DEF=32768 -Os"
LDFLAGS="-Wl,--gc-sections -Wl,-Map,hello.map -nostartfiles -nostdlib -L/home/aportnoy/Projects/riscv/freedom-e-sdk/bsp/sifive-hifive1-revb/install/lib/release/ -T/home/aportnoy/Projects/riscv/freedom-e-sdk/bsp/sifive-hifive1-revb/metal.default.lds"
LDLIBS="-Wl,--start-group -lc -lgcc -lm -lmetal -lmetal-gloss -Wl,--end-group"

riscv64-unknown-elf-gcc
-march=rv32imac -mabi=ilp32 -mcmodel=medlow
-ffunction-sections -fdata-sections
-I/home/aportnoy/Projects/riscv/freedom-e-sdk/bsp/sifive-hifive1-revb/install/include --specs=nano.specs
-DMTIME_RATE_HZ_DEF=32768
-Os
-Wl,--gc-sections
-Wl,-Map,hello.map
-nostartfiles
-nostdlib
-L/home/aportnoy/Projects/riscv/freedom-e-sdk/bsp/sifive-hifive1-revb/install/lib/release/
-T/home/aportnoy/Projects/riscv/freedom-e-sdk/bsp/sifive-hifive1-revb/metal.default.lds
hello.c
-Wl,--start-group
-lc
-lgcc
-lm
-lmetal
-lmetal-gloss
-Wl,--end-group
-o hello


Hmm, well I'm trying to load 0xdeadbeef into a register. I can load the top 20
bits first, then add another 12 bits, but they are sign extended. So I need to
find a value that can go into the upper 20 bits, such that when a 12 bit value
is added, I get 0xdeadbeef. The upper 20 bits are 0xdeadb. I could instead store
0xdeadc, then I want
  0xdeadc
    - 0x1000
    + 0x0eef
= 0xdeadbeef.

So the question is, what is this value 0x0eef - 0x1000 = -(0x1000 - 0x0eef)
= -0x0111 = 0xfeee.
So basically, we add 1 to the top 20 bits, and then try to find the two's
complement of the value we want to write to the low 12 bits. We then add that
value.
Would be nice to be able to write to the console. How to do that?
Let's just try blasting shit to txdata.
That worked in fact.

Let's try to write a routine that calculates fibonacci numbers and writes them
to memory.

Why the hell is pin 15 always on?
Looks like the GPIO memory map table is incorrect, two registers are missing:
0x38 IOF_EN
0x3C IOF_SEL

Now the value of IOF_SEL is all 0's, but IOF_EN has value 0x00030238.
 3  | 0  | 2  | 3  | 8
0011|0000|0010|0011|1000
            9    54 3
bits 3, 4, 5, 9, 16, 17 are set to perform some hardware function instead of
GPIO. 9 corresponds to 15, that's why it's always on.
3, 4, 5, 9 perform some SPI functions
16, 17 are UART 0 RX and TX
Let's try to disable pin 9. To do this I need to clear bit 9 in IOF_EN.

Why is 22 on a little bit? I see that in theory that's how the builtin LEDs are
connected, but then why is there a pin for that?
Let's look at the registers that I have. Specifically input_en, output_en,
iof_en. I want all my pins to be in output mode and not have iof_en set.

My pins are 2-9 printed or 18-23, 0-1 GPIO.

 2  3  4  5  6  7  8  9
18 19 20 21 22 23  0  1

The mask is then
31:28 27:24 23:20 19:16 15:12 11:8   7:4  3:0
0000  0000  1111  1100  0000  0000  0000 0011
   0     0     f     c     0     0     0    3

0x00fc0003

The weird pin is 6/22. If it were set, we'd see 0x00400000.

input_en:  0x00000400
output_en: 0x00030000
iof_en:    0x00030238

Pin 10 is set to be an input. Why?
Pins 16 and 17 are set to be outputs. What are they? They are UART RX and TX.
Pins 3, 4, 5, 9, 16, 17 perform hardware functions. There is no overlap with the
stuff I'm using.

Ok, when I'm sharing the builtin LED pins, they are on when the external ones
are off, and vice versa. I don't want this weird sharing. Let's remap 3, 5, 6.
I'm interested in 10-13. What's up with those pins.

Another discovery today was that it is possible to check whether the outgoing
UART FIFO is full. I now check for that before I print a character, now output
works correctly.
Also GNU screen has some weird setting that doesn't respect newlines (or
carriage returns?). It also doesn't respect the settings provided via stty, in
particular onclr or something. So I'm printing newlines on the board, and
converting newlines to carriage returns + newlines using a command line option
to this other tool called picocom:
```
sudo picocom /dev/ttyACM0 --baud 115200 --imap lfcrlf
```

I need to figure out linking so I can use global symbols and compile files
separately, so I can start building a library.

How do I make sure that a loop sleeps for exactly some amount of time?

Ok, I want to implement reading a button press. To do this, 
- figure out which pin to use
- enable the builtin pullup resistor.
- enable input on the pin
- read the value

Can I use GPIO 2, which is pin 10? Maybe, but it was weirdly set to always be an
input. Let's first try something that is less likely to fail. How about dig 15,
which is GPIO 9? Nope, it was set to do some hardware function. Let's do dig 16,
which is GPIO 10. It should have an internal power supply and pull up resistor.
So all I need to do is to provide a button and a ground.
Actually, GPIO 2 should be fine. GPIO 10 was the weird one, not DIG 10.

The button seems to be working. Now how do I detect a button press? A button
press is when a button is first detected to be pressed, then depressed.
Basically a button press is a transition from pressed to depressed.
