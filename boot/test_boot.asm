[org 0x7C00]
bits 16

; Init serial port COM1 (16550 UART)
mov dx, 0x3F8
mov al, 0x00
inc dx          ; IER port
out dx, al      ; IER = 0 (disable interrupts)
add dx, 2       ; LCR port (IER+2)
mov al, 0x80
out dx, al      ; LCR = 0x80 (enable DLAB)
sub dx, 3       ; DLL port (LCR-3)
mov al, 0x01
out dx, al      ; DLL = 1 (115200 baud @ 1.8432MHz)
inc dx          ; DLM port
mov al, 0x00
out dx, al      ; DLM = 0
add dx, 2       ; LCR port (DLM+2)
mov al, 0x03
out dx, al      ; LCR = 0x03 (8N1)
dec dx          ; FCR port (LCR-1)
mov al, 0xC7
out dx, al      ; FCR = 0xC7 (enable FIFO)
add dx, 2       ; MCR port (FCR+2)
mov al, 0x0B
out dx, al      ; MCR = 0x0B (RTS/DTR)

; DX = 0x3FC (MCR). Set DX back to DATA port (0x3F8)
sub dx, 4

; Write "Hi" to serial port
mov al, 'H'
out dx, al
mov al, 'i'
out dx, al
mov al, 0x0D
out dx, al
mov al, 0x0A
out dx, al

; Write to VGA text buffer (0xB8000)
mov ax, 0xB800
mov es, ax
mov word [es:0], 0x0F48  ; 'H' white on blue
mov word [es:2], 0x0F69  ; 'i' white on blue

; Write "Plan 0" to serial
mov si, msg
mov dx, 0x3F8
.repeat:
lodsb
or al, al
jz .done
push dx
add dx, 5       ; LSR port
.wait:
in al, dx
test al, 0x20
jz .wait
pop dx
mov al, [si-1]
out dx, al
jmp .repeat
.done:

; Halt
cli
.halt:
hlt
jmp .halt

msg db "Plan 0 Boot Sector OK", 0x0D, 0x0A, 0

times 510-($-$$) db 0
dw 0xAA55
