format elf64 executable
entry start

start:
	mov	rbp, rsp
	mov	rsp, f_stack
	mov	rdi, f_heap
	call	main
	xchg	rsp, rbp
	mov	edi, eax
	mov	eax, 60
	syscall


; Forth subroutines will use the following convention:
;
;	rax: volatile, cached TOS
;	rbx: volatile, scratch reg
;	rdx: volatile, cached NOS
;
;	rcx: callee-saved, loop counter / compiler state
;	rdi: callee-saved, string dest / global compile dest
;	rsi: callee-saved, string src / last compile src
;
;	rsp: volatile, data stack (hardware push/pop)
;	rbp: volatile, return stack (sub+mov / mov+add)
;
; The volatility of other registers is undefined.


; Enter and Exit macros for Forth words written in assembly
macro ENTER {
	; Push return address to stack at rbp
	lea	rbp, [rbp-8]
	pop	qword [rbp]
}
macro EXIT {
	; Pop return address from stack at rbp and return
	push	qword [rbp]
	lea	rbp, [rbp+8]
	ret
}

macro INLINER macr*, lbl* {
	lbl:
		mov	ecx, lbl#.end - lbl#.code
		call	inliner_stub
		; ^ Return address points to code.
		; It is consumed and will not return.
	.code:
		macr
	.end:
}
inliner_stub:
	pop	rsi
	rep movsb
	ret

INLINER ENTER, f_enter
INLINER EXIT, f_exit


;	S T A C K   O P E R A T I O N S

macro DROP {
	mov	rax, rdx
	pop	rdx
}
macro DUP {
	push	rdx
	mov	rdx, rax
}
macro SWAP {
	xchg	rax, rdx
}
macro OVER {
	push	rdx
	xchg	rax, rdx
}
macro NIP {
	pop	rdx
}
macro TUCK {
	push	rax
}

INLINER DROP, f_drop
INLINER DUP, f_dup
INLINER SWAP, f_swap
INLINER OVER, f_over
INLINER NIP, f_nip
INLINER TUCK, f_tuck


;	A R I T H M E T I C

macro ADD {
	add	rax, rdx
	pop	rdx
}
macro UM_ADD {
	add	rdx, rax
	sbb	eax, eax
	and	eax, 1
}
macro DEC {
	dec	rax
}

INLINER ADD, f_add
INLINER UM_ADD, f_um_add
INLINER DEC, f_dec


;	M E M O R Y   O P E R A T I O N S

macro FETCH {
	mov	rax, qword [rax]
}
macro STORE {
	mov	qword [rax], rdx
	pop	rax
	pop	rdx
}
macro CFETCH {
	movzx	eax, byte [rax]
}
macro CSTORE {
	mov	byte [rax], dl
	pop	rax
	pop	rdx
}

INLINER FETCH, f_fetch
INLINER STORE, f_store
INLINER CFETCH, f_cfetch
INLINER CSTORE, f_cstore


;	L O G I C   O P E R A T I O N S

macro OP2 name*, inst* {
	macro name \{
		inst	rax, rdx
		pop	rdx
	\}
	INLINER name, f_#inst
}
macro ZLT {
	sar	rax, 63
}

OP2 AND, and
OP2 OR, or
OP2 XOR, xor
INLINER ZLT, f_zlt


;	R E T U R N   S T A C K

macro TO_R {
	lea	rbp, [rbp-8]
	mov	qword [rbp], rax
	DROP
}

macro R_FETCH {
	push	rdx
	mov	rdx, rax
	mov	rax, qword [rbp]
}

macro R_FROM {
	R_FETCH
	lea	rbp, [rbp+8]
}

INLINER TO_R, f_to_r
INLINER R_FETCH, f_r_fetch
INLINER R_FROM, f_r_from


;	B R A N C H I N G   W O R D S

f_branch:
	ENTER
	mov	byte [rdi], 0xe9
	inc	rdi
	jmp	put_offset

f_qbranch:
	ENTER
	mov	dword [rdi], 0x00f88348		; cmp	rax, 0
	lea	rdi, [rdi+4]
	call	f_drop
	mov	word [rdi], 0x840f		; jz
	lea	rdi, [rdi+2]
	jmp	put_offset

f_call:
	ENTER
	mov	byte [rdi], 0xe8
	inc	rdi
put_offset:
	lea	rdi, [rdi+4]
	sub	rax, rdi
	mov	dword [rdi-4], eax
	DROP
	EXIT


;	I N N E R   I N T E R P R E T E R S

f_dolit:
	ENTER
	call	f_dup
	mov	word [rdi], 0xb848	; movabs rax
	lea	rdi, [rdi+2]
	mov	qword [rdi], rax
	lea	rdi, [rdi+8]
	DROP
	EXIT

macro EXECUTE {
	mov	rbx, rax
	DROP
	call	rbx
}

INLINER EXECUTE, f_execute


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	T E S T   P R O G R A M
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

macro FPUSH x* {
	push	rdx
	if x eq rdx
		xchg	rdx, rax
	else
		mov	rdx, rax
		if ~(x eq rax)
			mov	rax, x
		end if
	end if
}

f_begin:
	ENTER
	FPUSH	rdi
	EXIT

f_until:
	jmp	f_qbranch

main:
	ENTER

	FPUSH	10
	FPUSH	rdi

	call	f_enter
	FPUSH	0
	call	f_dolit
	call	f_swap
	call	f_begin
	call	f_tuck
	call	f_add
	call	f_swap
	call	f_dec
	call	f_dup
	call	f_zlt
	call	f_qbranch
	call	f_drop
	call	f_exit

	EXECUTE

	EXIT


; TODO Need a dictionary structure
; Put a type marker in front of the word before compilation.
;   0  -> "interpreted" (i.e. execute immediately and reset rdi)
;   1+ -> TBD
; TODO Need an I/O mechanism
; TODO Need an interpreter

f_heap:
	rq 1024
f_stack:
