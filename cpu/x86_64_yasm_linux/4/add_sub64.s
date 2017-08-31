; make the add function visible to the linker
; a leaf function demo

section .data
  msg    db  'Hello SubFunction', 0Ah  ; a message to print

section .text
         global _add
	     global _double
         global _print

;prototype: int64_t add(int64_t v1, int64_t v2) />
;desc: adds two integers and returns the result
;       RDI <-- v1
;       RSI <-- v2
;  calling convention:
;       first 6 parameters are in registers
;       the rest are following cdecl on stack[rsp]
;       return address FOLLOWED by saved rbp are always on stack
_add:
    push rbp
    mov  rbp, rsp

    push r12   ; persistent r12 - r15 accross calls
    push  rsi  ; pass the second parameter on stack
    call _double2
    pop  rsi  ; clean the stack, rcx is not used
    mov  r12,  rax
    
    push rdi  ; pass the 1st parameter 
    call _double2
    add  rax, r12
    pop  rdi   ; clear the stack usd by rdi, do not care the value
    pop  r12   ; restore rcx 
    call _print

    leave             ; restore (rbp, rsp)
    ret               ; return from sub-routine

_add2:
    push rbp
    mov  rbp, rsp

    push rcx
    mov  rax, rsi  ; get the 2nd parameters off the stack
    call _double
    mov  rcx,  rax
    mov  rax,  rdi
    call _double
    add  rax, rcx
    pop  rcx
    call _print

    leave             ; restore (rbp, rsp)
    ret               ; return from sub-routine

_print:
    push    rbp
    mov     rbp, rsp

    push    rax
    mov     edx, 18     ; number of bytes to write - one for each letter plus 0Ah (line feed character)
    mov     ecx, msg    ; move the memory address of our message string into ecx
    mov     ebx, 1      ; write to the STDOUT file
    mov     eax, 4      ; invoke SYS_WRITE (kernel opcode 4)
    int     80h
    pop     rax

    leave
    ret

; function _double
;   returns rax <- rax + rax, no overflow assumed
_double:
    push    rbp
    mov     rbp, rsp

    add rax, rax
    
    leave
    ret

; function _double2
; just to demo how to pass parameters via stack brutelly
;    input:  64 bit value on stack
;    return: 64 bit value * 2 --> rax
_double2:
    mov rax, qword [rsp + 8]
    push rbp
    mov  rbp, rsp

    add rax, rax

    leave
    ret
    
