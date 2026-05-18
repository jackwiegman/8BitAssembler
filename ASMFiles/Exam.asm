; Assembler Exam
; Function 1: A - determine category, returns category value;
; Function 2: B - Add to array, adds 1 to [bx+category]
;
; Variable addresses:
; [userInput]: C: 90
; []
;
; Put category boundaries in cx/dx 
; 0
get
cmp ax 0
jb [16]
; 0-4
;
mov [90] ax
; 5-6
; Determines category and adds to array
; 7-11
fun [26] 1 [90]
; [RET_ADDR1]: 
; 11:[RET_ADDR1]
mov bx [11]
; 12-13
;
jmp [0]
; 14-15
;
; Output each array part 
mov ax [bx]
put
; 16-17
mov ax [bx+1]
put
; 18-20
mov ax [bx+2]
put
; 21-23
;
; [HALT]:24
halt
;
; Determine category
; [ParamAddr]: 25

mov bx [25]
mov bx [bx+1]
mov ax [bx]
; 26-30
mov cx 0
; 31-32
mov bx 100
; 33-34
; move array address into bx
;
; Smaller than 10, jump to return
cmp ax 10
jb [47]
; 35-38
add cx 1
;39-40
cmp ax 30
jbe [47]
; 41-44
add cx 1
; 45-46
;
; after cx with category is set, put ax/bx at array address,
; then increment bx by category value and add one to that location of array.
; [Add and return]: 47
mov ax bx
add bx cx
;47-48
; Add 1 to array location of category.
mov dx [bx]
add dx 1
mov [bx] dx
;
; 49-52
; ax with array address returned
ret
;53
; [Array]: 100