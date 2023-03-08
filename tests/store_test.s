.data
bytes: .byte 0, 0
halfwords: .word16 0, 0
words: .word32 0, 0
dwords: .word 0, 0 
data: .word 100000, -100000, 100000000000, -100000000000

.text
daddui r1, r0, 100
daddi r2, r0, -100
;; bytes
xor r3, r3, r3
sb r1, bytes(r3)
daddui r3, r3, 1
sb r2, bytes(r3)

daddui r1, r0, 1000
daddi r2, r0, -1000
;; half
xor r3, r3, r3
sh r1, halfwords(r3)
daddui r3, r3, 2
sh r2, halfwords(r3)

daddui r4, r0, 0
lw r1, data(r4)
daddui r4, r4, 8
lw r2, data(r4)
;; words
xor r3, r3, r3
sw r1, words(r3)
daddui r3, r3, 4
sw r2, words(r3)

daddui r4, r0, 16
ld r1, data(r4)
daddui r4, r4, 8
ld r2, data(r4)
;; dwords
xor r3, r3, r3
sd r1, dwords(r3)
daddui r3, r3, 8
sd r2, dwords(r3)

halt