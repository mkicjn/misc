format elf64 executable
entry start

; System interface borrowed from paraforth
macro PUSHA [arg] {forward push arg}
macro POPA [arg] {reverse pop arg}
sys_exit:
	mov	edi, eax
	mov	eax, 60
	syscall

sys_tx:
	mov	[sys_xcv.mov+1], al
	mov	eax, 1
	jmp	sys_xcv
sys_rx:
	xor	eax, eax
sys_xcv:
	PUSHA rdi, rsi, rdx, rcx, r11
	mov	edi, eax
	lea	rsi, [sys_xcv.mov+1]
	mov	edx, 1
	syscall
	POPA rdi, rsi, rdx, rcx, r11
.mov:	mov	al, 127 ; self-modifying
	ret

; Integer printing
; (Recursive char-by-char solution, not ideal but easy)
putn: ; clobbers rax, rdx
	push	rbx
	mov	rbx, 10
	call	.rec
	pop	rbx
	ret
.rec:	xor	edx, edx
	div	rbx
	push	rdx
	cmp	rax, 0
	jz	.done
	call	.rec
.done:	pop	rax
	add	rax, 0x30 ; '0'
	call	sys_tx
	ret

collatz_next:
	lea	rdx, [2*rax + rax + 1]
	sar	rax, 1
	cmovc	rax, rdx
	ret

collatz: ; clobbers rax, rdx
	push	rcx
	xor	ecx, ecx
.loop:	cmp	rax, 1
	jle	.done
	push	rbx
	call	collatz_next
	pop	rbx
	inc	rcx
	jmp	.loop
.done:	mov	rax, rcx
	pop	rcx
	ret

maxlen: ; clobbers rax
	push	rbx
	mov	rcx, rax
	xor	ebx, ebx
.loop:	mov	rax, rcx
	call	collatz
	cmp	rbx, rax
	cmovl	rbx, rax
	loop	.loop
	mov	rax, rbx
	pop	rbx
	ret

start:
	mov	rbp, dstack
	mov	rax, 10000000
	call	maxlen
	call	putn
	xor	eax, eax
	call	sys_exit


	rq	1000
dstack:
