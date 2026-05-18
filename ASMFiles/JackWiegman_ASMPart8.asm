; Program that will add up 10 numbers entered by the user.
; Numbers must be between 30-70.
; Invalid numbers will not count towards total and
; will reprompt without showing current total 
;
; AX:   Input num to add, total output
; BX:   Hold total
; CX:   Holds count of numbers added so far
; DX:   Total numbers to add, compared to cx to check count
;
; put count limit in dx
mov dx 10
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
jae [29]
;
; get number from user
get 
mov [57] ax 
; 9-10
;
; check number between 30-70
fun [36] 1 [57]
; 15:[retAddr]
;
mov ax [15]
; 16-17
; function return 0 if not in range
cmp ax 0
je [5]
; 18-21
;
; add input to total & increment count
add bx ax 
add cx 1 
;
; put total into output register, then output current total
mov ax bx
put
; 25-26
;
; jump back to top of loop
jmp [5]
;
; [endLoop]:29
mov [57] bx 
;
; move total into ax then output
mov ax [57]
put
;
halt

; 35:[paramAddr]
; 36:[funcAddr]
mov bx [35]
mov bx [bx+1]
mov ax [bx]
; 36-40
;
; check 30 <= ax <= 70
cmp ax 30
;
jb [51]
cmp ax 70
ja [51]
; 41-48
;
; valid, ax already has valid number, just return
ret

; if invalid return 0
; [invalid]:51
mov ax 0
ret
;
;
; [storage] : 56, leave address after halt
