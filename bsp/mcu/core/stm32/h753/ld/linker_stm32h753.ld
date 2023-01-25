MEMORY
{
    FLASH_BANK_1 (rx) : ORIGIN = 0x08000000, LENGTH = 1024K
    FLASH_BANK_2 (rx) : ORIGIN = 0x08100000, LENGTH = 1024K
    ITCM_RAM (x)      : ORIGIN = 0x00000000, LENGTH = 64K
    DTCM_RAM (rw)     : ORIGIN = 0x20000000, LENGTH = 128K
    AXI_SRAM (rw)     : ORIGIN = 0x24000000, LENGTH = 512K
    SRAM_123 (rw)     : ORIGIN = 0x30000000, LENGTH = 288K
    SRAM_4 (rw)       : ORIGIN = 0x38000000, LENGTH = 64K
    BKPRAM (rw)       : ORIGIN = 0x38800000, LENGTH = 4K
}

ENTRY(Reset_Handler)

_HeapStart = ORIGIN(AXI_SRAM);
_HeapSize = LENGTH(AXI_SRAM);
_HeapLimit = ORIGIN(AXI_SRAM) + _HeapSize;

_HeapExtraStart = ORIGIN(DTCM_RAM);
_HeapExtraSize = LENGTH(DTCM_RAM);
_HeapExtraLimit = ORIGIN(DTCM_RAM) + _HeapExtraSize;

_estack = ORIGIN(DTCM_RAM) + LENGTH(DTCM_RAM);

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
        KEEP (*(.init))
        KEEP (*(.fini))
        . = ALIGN(4);
        *(.rodata*)
      } > FLASH_BANK_1

    .text_2 :
    {
        *(.text*)
        . = ALIGN(4);
        *(.rodata*)
      } > FLASH_BANK_2

    .init_section :
    {
        . = ALIGN(4);
        _sinit = .;
        KEEP(*(.init_array*))
        _einit = .;
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

    _sidata = LOADADDR(.data);

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > AXI_SRAM AT > FLASH_BANK_2

    . = ALIGN(4);
    .bss :
    {
        _sbss = .;
        __bss_start__ = _sbss;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
        __bss_end__ = _ebss;
    } > AXI_SRAM

    ._user_heap :
    {
        . = ALIGN(8);
        PROVIDE ( end = . );
        PROVIDE ( _end = . );
        _start_heap = .;
        . = _HeapSize - _ebss;
        . = ALIGN(8);
    } > AXI_SRAM

    /DISCARD/ :
    {
        libc.a ( * )
        libm.a ( * )
        libgcc.a ( * )
    }

    .ARM.attributes 0 :
    {
        *(.ARM.attributes)
    }
}
