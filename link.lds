TARGET(elf32-littleriscv)
ENTRY(raminit)
MEMORY
{
    ram (arw!xi) : ORIGIN = 0x80000000, LENGTH = 0x4000
    rom (irx!wa) : ORIGIN = 0x20010000, LENGTH = 0x6a120
}
SECTIONS
{
	. = 0x20010000;
	.text :
	{
		*(.text.raminit)
		*(.text .text*)
	} >rom
	.rodata : { *(.rodata .rodata*) } >rom
	_raminit = .;
	. = 0x80000000;
	_ramstart = .;
	.data :  { *(.data .data*)   } >ram AT>rom
	.sdata : { *(.sdata .sdata*) } >ram AT>rom
	.bss :   { *(.bss .bss*)     } >ram AT>rom
	.sbss :  { *(.sbss .sbss*)   } >ram AT>rom
	_ramend = .;
}
