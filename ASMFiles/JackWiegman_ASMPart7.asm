; Program that will add up 10 numbers entered by the user.
; Store the result in a memory location and output the total to the screen.
; You must use a loop.
;
; AX:   Input num to add, total output
; BX:   Hold total
; CX:   Holds count of numbers added so far
; DX:   Total numbers to add, compared to cx to check count
;
; put count limit in dx
mov dx 10 ; 0-1
;
; put actual count in cx, start at 0
mov cx 0 
;
; bx will hold total, initialize at 0
mov bx cx 
;
; compare count to total we're taking in
; 5 : [startLoop]
cmp cx dx 
;
; jump to end if cx >= dx
jae [16]
;
; get number from user
get
;
; add input to total & increment count
add bx ax 
add cx 1 
;
; put total into output register, then output current total
mov ax bx
put 
;
; jump back to top of loop
jmp [5] 
;
; [endLoop] 
mov [23] bx
;
; move total into ax then output
mov ax [23] 
put 
;
halt 
;
; [storage] : 23, leave address after halt
