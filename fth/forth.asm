format elf64 executable
entry start

start:
	mov	rbp, rsp
	mov	rsp, f_stack
	mov	rdi, f_heap
	mov	rsi, f_dict
	call	main
	xchg	rsp, rbp
	mov	edi, eax
	mov	eax, 60
	syscall


; Forth subroutines will use the following convention:
;
;	rax: volatile, cached TOS
;	rbx: volatile, scratch register
;	rdx: volatile, cached NOS
;
;	rcx: callee-saved, loop counter / compiler state (?)
;	rdi: callee-saved, string dest / global compile dest
;	rsi: callee-saved, string src / last compile src (?)
;
;	rsp: volatile, data stack (hardware push/pop)
;	rbp: volatile, return stack (sub+mov / mov+add)
;
; The volatility of other registers is undefined.

;	Entering and Exiting Forth words:
;
; Forth requires at least two stacks. x86 only supports one.
; Also, would like to use push/pop instructions for data stack.
; Solution: Manually place return addresses on another stack.
;
; ENTER and EXIT are a subroutine prologue/epilogue pair responsible for this.
; Note that not every subroutine needs to use ENTER and EXIT.
; Rule of thumb:
; * If you need to modify the data stack, use ENTER first.
; * If you use ENTER, then you should use EXIT to return.

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

;	D I C T I O N A R Y

macro DICTIONARY {
	dq 0
}
macro CNTSTR str* {
	local end
	db end-$-1
	db str
	end:
}
macro DICT_ADD lbl*, str* {
	macro DICTIONARY \{
		display	str, 10
		dq	lbl
		CNTSTR	str
		DICTIONARY
	\}
}

find:
	ENTER
	push	rdx		; DUP
	mov	rdx, rax	; ^
	push	rcx
	push	rsi
	push	rdi
.check_entry:
	mov	rax, rsi
	cmp	qword [rsi], 0
	jz	.return
	lea	rsi, [rsi+8]
	movzx	ecx, byte [rsi]
	inc	rcx
	repe cmpsb
	je	.return
.advance:
	mov	rdi, [rsp]
	add	rsi, rcx
	jmp	.check_entry
.return:
	mov	rax, [rax]
	pop	rdi
	pop	rsi
	pop	rcx
	EXIT


;	I N L I N E R   G E N E R A T O R

macro INLINER macr*, lbl*, str* {
	DICT_ADD lbl, str
	lbl:
		push	qword lbl#.end-lbl#.code
		call	inliner_stub
		; ^ Return address points to code.
		; It is consumed and will not return.
	.code:
		macr
	.end:
}
inliner_stub:
	; Stash preserved values under the return stack
	mov	[rbp-8], rsi
	mov	[rbp-16], rcx
	; Retrieve arguments from stack and copy
	pop	rsi
	pop	rcx
	rep movsb
	; Retrieve stashed values
	mov	rcx, [rbp-16]
	mov	rsi, [rbp-8]
	ret

INLINER ENTER, f_enter, 'enter'
INLINER EXIT, f_exit, 'EXIT'


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

INLINER DROP, f_drop, 'DROP'
INLINER DUP, f_dup, 'DUP'
INLINER SWAP, f_swap, 'SWAP'
INLINER OVER, f_over, 'OVER'
INLINER NIP, f_nip, 'NIP'
INLINER TUCK, f_tuck, 'TUCK'


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

INLINER ADD, f_add, '+'
INLINER UM_ADD, f_um_add, 'UM+'
INLINER DEC, f_dec, '1-'


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

INLINER FETCH, f_fetch, '@'
INLINER STORE, f_store, '!'
INLINER CFETCH, f_cfetch, 'C@'
INLINER CSTORE, f_cstore, 'C!'


;	L O G I C   O P E R A T I O N S

macro OP2 name*, inst*, str* {
	macro name \{
		inst	rax, rdx
		pop	rdx
	\}
	INLINER name, f_#inst, str
}
macro ZLT {
	sar	rax, 63
}

OP2 AND, and, 'AND'
OP2 OR, or, 'OR'
OP2 XOR, xor, 'XOR'
INLINER ZLT, f_zlt, '0<'


;	R E T U R N   S T A C K

macro TO_R {
	lea	rbp, [rbp-8]
	mov	qword [rbp], rax
	DROP
}

macro R_FETCH {
	DUP
	mov	rax, qword [rbp]
}

macro R_FROM {
	R_FETCH
	lea	rbp, [rbp+8]
}

INLINER TO_R, f_to_r, '>R'
INLINER R_FETCH, f_r_fetch, 'R@'
INLINER R_FROM, f_r_from, 'R>'


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

DICT_ADD f_branch, 'branch'
DICT_ADD f_qbranch, '?branch'


;	I N N E R   I N T E R P R E T E R S

f_dolit:
	ENTER
	call	f_dup
	mov	word [rdi], 0xb848		; movabs rax
	lea	rdi, [rdi+2]
	mov	qword [rdi], rax
	lea	rdi, [rdi+8]
	DROP
	EXIT

DICT_ADD f_dolit, 'dolit'

macro EXECUTE {
	mov	rbx, rax
	DROP
	call	rbx
}

INLINER EXECUTE, f_execute, 'EXECUTE'


;	C O M P I L E R

include 'lin64io.asm'

parse_name:	; TODO: Test this subroutine
	push	rax
	push	rcx
	xor	rcx, rcx
.sp:	; Skip whitespace
	call	rx_byte
	cmp	al, 0x20
	jle	.sp
	; Store the word characters
	inc	rdi
.l:	stosb	
	inc	rcx
	call	rx_byte
	cmp	al, 0x20
	jg	.l
	; Count the word
	sub	rdi, rcx
	dec	rdi
	mov	[rdi], cl
	; Return
	pop	rcx
	pop	rax
	ret

makedef:
	; copy counted string at rdi to space under rsi
	; preserve rdi and rcx; point rsi to copied string
	push	rcx
	push	rdi
	movzx	ecx, byte [rdi]
	inc	ecx		
	sub	rsi, rcx
	push	rsi
	xchg	rdi, rsi
	rep movsb
	xchg	rdi, rsi
	pop	rsi
	pop	rdi
	pop	rcx
	; store current heap ptr under rsi, modifying rsi
	lea	rsi, [rsi-8]
	mov	[rsi], rdi
	ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	T E S T   P R O G R A M
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

macro FPUSH x* {
	push	rdx
	if x eq rdx
		xchg	rdx, rax
	else
		mov	rdx, rax
		if ~ x eq rax
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

	call	rx_byte
	call	tx_byte

;	FPUSH	10
;	FPUSH	rdi
;	call	f_enter
;	FPUSH	0
;	call	f_dolit
;	call	f_swap
;	call	f_begin
;	call	f_tuck
;	call	f_add
;	call	f_swap
;	call	f_dec
;	call	f_dup
;	call	f_zlt
;	call	f_qbranch
;	call	f_drop
;	call	f_exit
;	EXECUTE

	EXIT


; Dictionary idea: All words are immediate by default.
; Entries only contain a string (how?) and pointers to code.
; Code is executed immediately on invocation.
; The whole dictionary will reside in its own memory area.

; Interpreter idea:
; Manipulate the return stack to decide whether to interpret or not.
; `:` simply drops the address of the interpreted code from the stack.
; `;` simply does RDROP to exit the interpreter and run.
; Another return to the interpreter will be underneath.

; TODO Need a macro to compile a function call.
; TODO Need a function to compile the above macro into memory
; TODO Need an I/O mechanism
; TODO Need an interpreter (see interpreter.txt in notes)
; TODO Tail recursion check by `;` (last instruction: call -> jmp)
; TODO Rewrite as much of this in Forth as possible, hopefully most :)


; 	Memory map
;
; | 0		<- Dict | Heap ->	<- Stack |	<- Return stack|
;

	rq 8
f_dict:
	DICTIONARY
f_heap:
	rq 1024
f_stack:
