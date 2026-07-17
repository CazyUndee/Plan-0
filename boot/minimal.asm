; Minimal multiboot test for QEMU a.out kludge - flat binary
ORG 0x100000
BITS 32

; Multiboot header (a.out kludge)
align 4
header_start:
    dd 0x1BADB002              ; magic
    dd 1 << 16                 ; flags: a.out kludge
    dd -(0x1BADB002 + (1 << 16)) ; checksum
    dd header_start            ; header_addr (points to magic!)
    dd 0x100000                ; load_addr
    dd 0                       ; load_end_addr (0 = entire file)
    dd 0                       ; bss_end_addr (0 = no extra)
    dd _start                  ; entry_addr

; Pad to known offset
times 64-($-$$) db 0

_start:
    ; Write 'H' to serial port COM1
    mov dx, 0x3F8
    mov al, 'H'
    out dx, al

    ; Write 'i' to serial port  
    mov al, 'i'
    out dx, al

    ; Write newline to serial
    mov al, 0x0D
    out dx, al
    mov al, 0x0A
    out dx, al

    ; Also write to VGA text buffer
    mov dword [0xB8000], 0x0F210F48  ; "Hi!" white on blue

    ; Halt
    cli
.halt:
    hlt
    jmp .halt
