%ifdef __i386__

%define VIDEOMODE	1

[BITS 32]

extern load_bootargs
extern mbd_grub
extern main
extern text
extern bss
extern end


MALIGN equ 		(1 << 0)
MEMINFO equ		(1 << 1)
VMODE equ		(1 << 2)
FLAGS equ		(MALIGN | MEMINFO | VMODE)
MAGIC equ		(0x1BADB002)
CHECKSUM equ	-(MAGIC + FLAGS)
STACKSIZE equ	0x4000



global _start
global kernel_stack
global kernel_stack_end
global multiboot



section .bss
align 0x1000

kernel_stack: resb STACKSIZE
kernel_stack_end:

align 0x1000
	
section .multiboot
multiboot:
align 4
	dd MAGIC
	dd FLAGS
	dd CHECKSUM
	dd multiboot
	dd text
	dd bss
	dd end
	dd _start
%if VIDEOMODE
	dd 0
	dd 1024
	dd 768
	dd 32
%else
	dd 1
	dd 0
	dd 0
	dd 0
%endif
		
		
	
section .text
_start:
	mov [mbd_grub], ebx
	mov esp, kernel_stack + STACKSIZE
	
	cli
	call enable_fpu
	call enable_sse
	call enable_kb
	call load_bootargs
	;call load_bss
	call main
	call reboot
	jmp $
	
	
reboot:
	cli
	mov cx, 0x02
	mov dx, 0x64
.L1:
	and cx, 0x02
	jz .L2
	
	in al, dx
	mov cx, ax
	jmp .L1
.L2:	
	mov al, 0xFE
	out dx, al
	jmp $
ret


enable_fpu:
	finit
	xor eax, eax
ret

enable_sse:	
	mov eax, cr0
	and ax, 0xFFFB
	or ax, 2
	mov cr0, eax
	mov eax, cr4
	or ax, 3 << 9
	mov cr4, eax
	xor eax, eax
ret

enable_kb:
	in al, 0x60
	xor eax, eax
ret

load_bss:
	xor eax, eax
	mov edi, bss
	mov ecx, end
	sub ecx, edi
	cld
	rep stosb
ret


%endif
	
