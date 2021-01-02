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

---
Goals:
[ ] use PLL for a higher frequency clock
	[ ] verify clock frequency
[ ] try to configure a timer + interrupt

I figured out that the clock is in fact 16 MHz. I will need to set it as input
to PLL. Ok, it works by running the following:
	prci->pllr = 0x1;
	prci->pllf = 0x3;
	prci->pllq = 0x1;
	prci->pllsel = 1;
	prci->pllbypass = 0;
Calculations:
pllr {0x1}: 16MHz/2 ->  8MHz
pllf {0x3}:  8MHz*8 -> 64MHz
pllq {0x1}: 64MHz/2 -> 32MHz

The problem is that UART baud rate needs to be adjusted accordingly on the
receiving side, so it's easier to just run the default 16 MHz clock and use the
115200 UART baud rate. For 320 MHz I would have to multiply the baud rate by
20?

---
Now I want to try to use a timer interrupt. Let's implement one to print the
current cycle every half a second or something.
Actually, first let's just confirm that it's working by printing something from
inside the interrupt.
- set mtvec to the interrupt handler address
  - make sure the address is 64 byte aligned
  - (thus setting the mode to direct)
- set mie.MTIE to enable machine timer interrupts
- set mtimecmp to some value
- set mstatus.MIE to enable interrupts globally
- inside the interrupt, make sure to save any registers we're going to be using
  is it possible to ensure this in C? yes, GCC supports an interrupt function
  that will generate a prologue/epilogue for the interrupt
  - print cause
  - print cycle
  - increment mtimecmp

I see that __attribute__ ((interrupt)) generates code to save a bunch of
registers. It saves too many. Can link time optimization fix this? GCC has it
and it can be enabled with the flag -flto.
Ok, let's just try to enable a timer interrupt. Sounds like CLINT will be
responsible for that.

Link time optimization really helped a lot! We're saving way fewer registers in
the interrupt handler and many functions are inlined.

---
Let's use a timer interrupt to check for button presses. Ideally this would be
done using external interrupts instead.
I was messing with a linker script to ensure that static global variables are
placed in memory instead of flash.
Ok, I got it to work by following instructions from
http://bravegnu.org/gnu-eprog/data-in-ram.html.

---
What's up with GPIO0? It seems to interfere with JTAG upload. I get errors like
these:
```
Connecting to target via JTAG                                                                  
ConfigTargetSettings() start                 
ConfigTargetSettings() end    
TotalIRLen = 4, IRPrint = 0x01                                                                 
JTAG chain detection found 1 devices:                                                          
 #0 Id: 0x00000001, IRLen: 04, Unknown device                                                  
RISC-V behind DAP detected                                                                     
DAP error while determining CoreSight SoC version
ConfigTargetSettings() start                                                                   
ConfigTargetSettings() end                                                                     
TotalIRLen = 4, IRPrint = 0x01             
JTAG chain detection found 1 devices:                                                          
 #0 Id: 0x00000001, IRLen: 04, Unknown device                                                  
RISC-V behind DAP detected                                                                     
DAP error while determining CoreSight SoC version
ConfigTargetSettings() start       
ConfigTargetSettings() end                                                                     
TotalIRLen = 4, IRPrint = 0x01              
JTAG chain detection found 1 devices:        
 #0 Id: 0x00000001, IRLen: 04, Unknown device                                                  
RISC-V behind DAP detected                                                                     
DAP error while determining CoreSight SoC version
ConfigTargetSettings() start                   
ConfigTargetSettings() end    
TotalIRLen = 4, IRPrint = 0x01       
JTAG chain detection found 1 devices:        
 #0 Id: 0x00000001, IRLen: 04, Unknown device
RISC-V behind DAP detected                                                                     
DAP error while determining CoreSight SoC version
Cannot connect to target.
```

---
I'm trying to figure out the protocol X that is built on top of SPI and used to
run AT commands on the Espressif. The ultimate goal is to use Bluetooth.
Ok, what SPI instance is used? What are the pin numbers for
- clock       -> GPIO5 aka DIG13
- MOSI        -> GPIO3 aka DIG11
- MISO        -> GPIO4 aka DIG12
- chip select -> GPIO9 aka DIG15

there's also some handshake pin:
- handshake   -> GPIO10 aka DIG16

I want to probe these using a logic analyzer and try to reverse engineer
protocol X.
Setup:
- hookup the wires to the logic analyzer probes
- run a simple empty program

I saved a log in esp32-spi-log.txt.

---
Actually I'll postpone communication for later. Working with the
gyro/accelerometer is more critical to the project.
Would be cool to start interfacing with it. I also need to figure out the math.
I want to estimate position using data from gyro + accelerometer.
Maybe for starters I can test a stationary sonar in 2D. So map angle (direction)
to a length.
Looks like the module I have uses I2C for communication.

2020-10-12
Ok, the current goal is to understand what data I get from the accelerometer
and the gyroscope and design an algorithm that converts timestamped
measurements to position history.
I found an official datasheet for MPU-6050 from InvenSense at this link:
https://www.cdiweb.com/datasheets/invensense/mpu-6050_datasheet_v3%204.pdf.
Apparently it uses I2C. Let's maybe read about the protocol?
I'm reading Elecia White:
- SCL is the clock
- SDA is data

Now what the heck are those other pins:
- XDA,
- XCL,
- ADO,
- INT?

Ok, right now I just want to find out what the data looks like and work on the
algorithm. Let's not worry so much about interfacing.
Holy crap, the datasheet mentions motion processing and sensor fusion.

Also a reminder, I saw that compass and altimeter/barometer are also very
useful for sensor fusion. Should probably look into getting those.  Hmm, turns
out MPU 9250 has those integrated. I also found some Adafruit absolute
orientation board that apparently can spit out orientation or position...  If
it can do that, then my problem is solved for me. Would it be fun to implement
from scratch?
Well, it seems like this integrated sensor can only provide orientation, not
position. In addition, it can only output linear acceleration at 100Hz max. I
think the higher the frequency at which I poll, the less error I get. I wonder
how often I can poll both the ultrasonic sensor and the gyro/accelerometer?
Maybe I can poll the ultrasonic sensor less often than the MPU. Or if I run the
clock at 320 Mhz, I can do a lot of work in between, and so timing should be
less of an issue.
Ok, let's just read the datasheet for the simpler MPU and see how often it can
provide data.
I also would like to understand what exactly is the source of drift. If I could simulate the process... And visualize.

Ok, looks like the extra pins are to allow communication with an external compass, or other sensors.

Alright, I can either start working out the algorithm or try to interface with
the thing.


# Wed Dec 30 04:08:51 PM PST 2020
Alrighty, let's try to bring up the BNO 055. I just want communication to work
and to be able to print stuff to serial.

## BNO 055
### Pinout
- Vin: 3.3-5.0 V
- GND
- (I2C) SCL
- (I2C) SDA

Now how does I2C work on my board? The core is some opensource implementation
which I should try to find and understand. Otherwise the address of the instance
is at.

### I2C
1. Master creates start condition on the 2 lines.
2. Master sends 7 address bits and R/W bit, MSB first.
3. Slave pulls SDA down to acknowledge it's listening.
4. Suppose master is writing, then master sends 8 bits of data.
5. Slave pulls SDA down to ACK.
6. Master creates stop condition.

#### General I2C flow specifics
1. Start signal
	- STA bit in the command register set
	- the RD or WR bits set
2. slave address transfer
	- store the slave device's address in the transmit register
	- set the WR bit
3. data transfer
	- store data in the transmit register
	- set wr
	- check TIP which indicates transfer in progress when set
	- check IF which indicates receive register contains valid data when set
	- can issue new write/read when TIP reset
4. Stop signal
	hmmm

#### Questions
- given 400 kHz, what should the prescaler equal?

#### BNO 055 I2C specifics
- clock:   400 kHz
- address: 0x29


#### Trial run
Ok, want to read some data from the device. The sequence is that we first send
the register address, then read the data.

1. Write slave address to TXR (shifted) ORed with 1, set STA and WR.
2. Wait for TIP and check ACK.
3. Write register address to TXR, set WR.
4. Wait for TIP and check ACK.
5. Write slave address to TXR (shifted) ORed with 0, set STA and RD.
6. Wait for TIP and check ACK.
7. Set the RD bit.
8. Wait for TIP and IF.
9. Read RXR.
10. Repeat 7-9 if necessary.
11. Send NACK.
12. Generate STOP by setting STO.


Alright, I just managed to get BNO 055 to ACK.
First, I had connectivity issues on the breadboard, so I had to solder the
headers.
Then I used the Arduino I had to verify that the device works correctly.

Can I just send a START reliably?
Develop a way to determine who is responsible for either line being down and how
it can be fixed.
Look at the communication pattern between Arduino and BNO 055.
