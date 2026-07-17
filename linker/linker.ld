ENTRY(_start)

SECTIONS
{
    /* Multiboot header + kernel at 1MB */
    . = 0x100000;
    
    .multiboot :
    {
        *(.multiboot)
    }

    /* Boot code */
    .boot :
    {
        *(.boot)
        *(.text)
        *(.text.*)
    }

    /* Read-only data */
    .rodata :
    {
        *(.rodata)
        *(.rodata.*)
    }

    /* Initialized data */
    .data :
    {
        *(.data)
        *(.data.*)
    }

    /* BSS (uninitialized data) */
    .bss :
    {
        bss_start = .;
        *(COMMON)
        *(.bss)
        *(.bss.*)
        bss_end = .;
    }

    /* Discard unwanted sections */
    /DISCARD/ :
    {
        *(.comment)
        *(.note.*)
        *(.eh_frame*)
    }

    kernel_end = .;
}
