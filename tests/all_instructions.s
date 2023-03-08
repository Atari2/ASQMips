.text
lb r1, 1(r1)
lbu r1, 1(r1)
sb r1, 1(r1)
lh r1, 1(r1)
lhu r1, 1(r1)
sh r1, 1(r1)
lw r1, 1(r1)
lwu r1, 1(r1)
sw r1, 1(r1)
ld r1, 1(r1)
sd r1, 1(r1)
l.d f1, 1(r1)
s.d f1, 1(r1)
halt
daddi r1, r1, 1
daddui r1, r1, 1
andi r1, r1, 1
ori r1, r1, 1
xori r1, r1, 1
lui r1, 1
slti r1, r1, 1
sltiu r1, r1, 1
beq r1, r1, 1
bne r1, r1, 1
beqz r1, 1
bnez r1, 1
j 1
jr r1
jal 1
jalr r1
dsll r1, r1, 1
dsrl r1, r1, 1
dsra r1, r1, 1
dsllv r1, r1, r1
dsrlv r1, r1, r1
dsrav r1, r1, r1
movz r1, r1, r1
movn r1, r1, r1
nop
and r1, r1, r1
or r1, r1, r1
xor r1, r1, r1
slt r1, r1, r1
sltu r1, r1, r1
dadd r1, r1, r1
daddu r1, r1, r1
dsub r1, r1, r1
dsubu r1, r1, r1
dmul r1, r1, r1
dmulu r1, r1, r1
ddiv r1, r1, r1
ddivu r1, r1, r1
add.d f1, f1, f1
sub.d f1, f1, f1
mul.d f1, f1, f1
div.d f1, f1, f1
mov.d f1, f1
cvt.d.l f1, f1
cvt.l.d f1, f1
c.lt.d f1, f1
c.le.d f1, f1
c.eq.d f1, f1
bc1f 1
bc1t 1
mtc1 r1, f1
mfc1 r1, f1
