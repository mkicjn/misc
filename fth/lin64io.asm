buf:
	dq 1

rx_byte:
	ENTER
	push	rdx
	mov	rdx, rax

	push	rcx
	push	rdx
	push	rdi
	push	rsi

	xor	eax, eax
	xor	edi, edi
	mov	rsi, buf
	mov	edx, 1
	syscall
	movzx	eax, byte [buf]

	pop	rsi
	pop	rdi
	pop	rdx
	pop	rcx
	EXIT
	ret

tx_byte:
	ENTER
	push	rdx
	mov	rdx, rax

	push	rcx
	push	rdx
	push	rdi
	push	rsi

	mov	eax, 1
	mov	edi, 1
	mov	rsi, buf
	mov	edx, 1
	syscall
	movzx	eax, byte [buf]

	pop	rsi
	pop	rdi
	pop	rdx
	pop	rcx
	EXIT
	ret
