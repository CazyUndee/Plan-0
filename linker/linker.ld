ENTRY(_start)

SECTIONS
{
    /* Multiboot header must be at the very beginning (offset 0) */
    . = 0;
    
    .multiboot :
    {
        *(.multiboot)
    }

    /* Load kernel at 2MB (leaves room for bootloader) */
    . = 0x200000;

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
