.686
.MODEL FLAT, C

.STACK

.DATA
    
.CODE


; For x86-32
SwitchContext PROC
    push ebp
    mov ebp, esp

    ; push old callee save registers
    push ebx
    push edi
    push esi

    ; save args to registers
    mov edi, [ebp+8]     ; from
    mov esi, [ebp+12]    ; to

    ; switch context
    mov [edi], esp    ; save current esp to from->esp_
    mov esp, [esi]    ; load to->esp_ to current esp

    ; pop new callee save registers
    
    pop esi
    pop edi
    pop ebx

    ; cleen local variables
    ;mov esp, ebp

    pop ebp
    ret
SwitchContext ENDP

END




