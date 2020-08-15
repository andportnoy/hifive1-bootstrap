TARGET(elf32-littleriscv)
ENTRY(main)
MEMORY
{
    ram (arw!xi) : ORIGIN = 0x80000000, LENGTH = 0x4000
    rom (irx!wa) : ORIGIN = 0x20010000, LENGTH = 0x6a120
}
SECTIONS
{
	. = 0x20010000;
	_romstart = .;
	.text :
	{
		*(.text.startup.main .text.startup*)
		*(.text .text*)
	} >rom
	. = 0x80000000;
	.data : { *(.data) } >ram AT>rom
}
