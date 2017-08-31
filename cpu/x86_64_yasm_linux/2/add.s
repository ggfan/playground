; make the add function visible to the linker
; a leaf function demo

section .text
         global _add

;prototype: int __cdcl add(int a, int b) />
;desc: adds two integers and returns the result
;      on the stack:
;            ret_address
;            right-most parameter
;            left-most parameter
_add:
    mov ecx, [esp+4]  ; get the 2nd parameters off the stack
    mov eax, [esp+8]  ; get the 1st parameter off the stack
    add eax, ecx      ; add the parameters, return value in eax
    ret               ; return from sub-routine

