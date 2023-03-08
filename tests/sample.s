.data
v1: .byte 3, 7
str: .asciiz "Hello World"
doubles: .double 1.2, 2.3
;; r1 = aa
;; r2 = bb

.text

daddui r10, r0, 0
daddui r6, r0, 3
daddui r3, r0, 5
daddui r7, r0, 5

l0:
andi r1, r10, 1
lb r2, v1(r1)
slt r1, r2, r3
bnez r1, l1
daddi r12, r0, 10
l1:
slt r4, r6, r7
bnez r4, l2
daddi r16, r0, 10
l2:
slt r3, r2, r7
beqz r3, l3
l.d f5, doubles(r0)
l3:
daddi r10, r10, 1
daddi r11, r10, -99
bnez r11, l0
halt