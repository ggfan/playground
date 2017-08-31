; make the add function visible to the linker
; a leaf function demo

section .data
  msg    db  'Hello SubFunction', 0Ah  ; a message to print

section .text
         global _add
	 global _double
         global _print

;prototype: int __cdecl add(int a, int b) />
;desc: adds two integers and returns the result
;      on the stack:
;            ret_address
;            right-most parameter
;            left-most parameter
_add:
    push ecx
    mov eax, [esp+8]  ; get the 2nd parameters off the stack
    call _double
    mov ecx, eax
    mov eax, [esp+12] ; get the 1st parameter off the stack
    call _double
    add eax, eax      ; add the parameters, return value in eax

    call _print
    pop ecx
    ret               ; return from sub-routine

_print:
    push    eax
    mov     edx, 18     ; number of bytes to write - one for each letter plus 0Ah (line feed character)
    mov     ecx, msg    ; move the memory address of our message string into ecx
    mov     ebx, 1      ; write to the STDOUT file
    mov     eax, 4      ; invoke SYS_WRITE (kernel opcode 4)
    int     80h
    pop     eax
    ret

; function _double
;   returns eax <- eax + eax, no overflow assumed
_double:
    add eax, eax
    ret

    
