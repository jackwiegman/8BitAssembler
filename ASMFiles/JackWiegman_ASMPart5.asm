put 
;
mov ax 25
put 
;
mov bx 34
mov cx 9999
mov dx 208
put
;
mov bx ax
put
;
add ax bx
put 
;
mov [50] ax
mov ax cx
add cx dx
put
;
mov [51] cx
mov ax cx
put
;
mov bx [51]
put 
;
;
mov bx dx 
put 
;
mov [52] dx 
put 
mov [53] ax 
put 
;
;
; Part 5 Test - Load values from data section,
; add, store result, output
; Data is stored at addresses 47, and 48. Result goes in 49.
;
; Load first value from address 47 into ax
mov ax [47] 
put 
; Load second value from address 48 into bx
mov bx [48]
put 
; Add them together (result in ax)
add ax bx 
put 
;
; Store result at address 49
mov [49] ax 
; Output the total
put 
;
halt 
;
; Data section
50 
30 

