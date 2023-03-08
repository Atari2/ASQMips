.data
bytes: .byte 3, -3
halfwords: .word16 3, -3
words: .word32 3, -3
dwords: .word 3, -3

.text
;; bytes
xor r3, r3, r3
lb r1, bytes(r3)
lbu r2, bytes(r3)
daddui r3, r3, 1
lb r4, bytes(r3)
lbu r5, bytes(r3)

;; half
xor r3, r3, r3
lh r6, halfwords(r3)
lhu r7, halfwords(r3)
daddui r3, r3, 2
lh r8, halfwords(r3)
lhu r9, halfwords(r3)

;; words
xor r3, r3, r3
lw r10, words(r3)
lwu r11, words(r3)
daddui r3, r3, 4
lw r12, words(r3)
lwu r13, words(r3)

;; dwords
xor r3, r3, r3
ld r14, dwords(r3)
daddui r3, r3, 8
ld r15, dwords(r3)

halt