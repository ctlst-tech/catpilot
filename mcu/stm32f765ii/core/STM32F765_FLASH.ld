MEMORY
{
    FLASH_BANK_1 (rx) : ORIGIN = 0x08000000, LENGTH = 1024K
    FLASH_BANK_2 (rx) : ORIGIN = 0x08100000, LENGTH = 1024K
    ITCM_RAM (x)      : ORIGIN = 0x00000000, LENGTH = 16K
    DTCM_RAM (rw)     : ORIGIN = 0x20000000, LENGTH = 128K
    SRAM_1 (rw)       : ORIGIN = 0x20020000, LENGTH = 368K
    SRAM_2 (rw)       : ORIGIN = 0x2007c000, LENGTH = 16K
}

ENTRY(Reset_Handler)

_estack = ORIGIN(ITCM_RAM) + LENGTH(ITCM_RAM);

SECTIONS
{
	.isr_vector :
	{
		KEEP(*(.isr_vector))
	} > FLASH_BANK_1

	.text_1 : 
	{
		*(.text*)
		. = ALIGN(4);
		*(.rodata*)
  	} > FLASH_BANK_1

  	.text_2 : 
	{
		*(.text*)
		. = ALIGN(4);
		*(.rodata*)
  	} > FLASH_BANK_2

	.ARM.extab : 
	{
	    . = ALIGN(4);
		*(.ARM.extab*)
	} > FLASH_BANK_2

	.ARM.exidx : 
	{
	    . = ALIGN(4);
		__exidx_start = .;
		*(.ARM.exidx*)
		__exidx_end = .;
	} > FLASH_BANK_2

	.init_section : 
	{
	    . = ALIGN(4);
		_sinit = .;
		KEEP(*(.init_array*)) 
		_einit = .;
	} > FLASH_BANK_2

	_sidata = LOADADDR(.data);

	.data : 
	{
		_sdata = .;
		*(.data*)
		. = ALIGN(4);
		_edata = .;
	} > SRAM1 AT > FLASH_BANK_2

	.bss : 
	{
		_sbss = .;
		*(.bss*) 
		*(COMMON)
		. = ALIGN(4);
		_ebss = .;
	} > SRAM1
}
