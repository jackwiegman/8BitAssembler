; Program that will add up 10 numbers entered by the user.
; Numbers must be between 30-70.
; The parameter passed will be the address of where to put
; the result of the function.
;
; AX:   Input num to add, total output
; BX:   Hold total
; CX:   Holds count of numbers added so far
; DX:   Total numbers to add, compared to cx to check count
;
; Problems: I think my main issue was I making sure ax and bx were set to the
; proper value. It was a bit of a struggle making sure addresses/literals were
; put in the right registers at the right times.
;
; initialize registers, dx:10(numInputer), cx:0(count), bx:0(total)
mov dx 10
mov cx 0
mov bx cx
; 0-4
; compare count to total we're taking in
;** [startInput]:5
cmp cx dx
;
; jump to end if cx >= dx
jae [37]
; 6-7
;
; get number from user
get
mov [68] ax
; 8-10
;
; check number between 30-70
; mem:11
fun [41] 1 [68]
; [retAddr]:15
; 11-15
;
; Return address has address of result.
; Put value in ax
mov ax [15]
mov bx ax
mov ax [bx]
; 16-19
;
; function return 0 if not in range
; line:20
cmp ax 0
je [5]
; 20-23
;
; Put total back in bx
mov bx [69] 
; 24-25
;
; add input to total & increment count
; line:26
add bx ax
add cx 1 
; 26-28
;
mov [69] bx 
; 29-30
;
; move count to ax and output
mov ax cx
put
; 31-32
;
; put total into output register, then output current total
; and jump to start of loop.
;
; line:33
mov ax bx
put
jmp [5]
;
; 33-36
;
;
;
; move total into ax then output and exit
;** [endAndOutput]:37
;
mov ax bx
put 
halt
; 37-39
; halt:39
; blank:40

;** [paramAddr]:40
;** [funcAddr]:41
;
; Put value in parameter address in ax
;
mov bx [40]
mov bx [bx+1]
mov ax [bx]
;
; 41-45
;
; check 30 <= ax <= 70
; line:46
cmp ax 30
jb [60]
cmp ax 70
ja [60]
;
; 46-53
;
; if valid, ax already has valid number, put in mem
; line:54-55
mov [70] ax
;
mov ax 70
ret

;
;
;
; if invalid, put 0 in mem location
;** [invalid]:60
mov ax 0
mov [70] ax 
;
; 60-63
;
; return mem location
; line:62
;
; 64-65
mov ax 70
ret

; ret:66-67
;
;** [storage]:68, address to store value for function to use.
;** [total]:69, address to store total sum.
;** [output]:70, address to store function output, address is returned
