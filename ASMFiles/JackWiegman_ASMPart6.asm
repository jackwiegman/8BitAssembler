; get 2 values from memory, put into bx and cx
mov bx [18] 
mov cx [19] 
;
; ax will hold total, starts at 0
mov ax 0 
;
; dx will be loop counter start at 0
mov dx ax
;
; initial values
put 
;
; adding bx to ax for #cx iterations
;
; compare count to muliplier
cmp dx cx 
; jump to output if count >= muliplier
jae [17] 
;
add ax bx
add dx 1
;
; [output]
put 
;
;
jmp [8]
;
halt
;
; [mem1]
5
; [mem2]
3
