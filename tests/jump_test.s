.text

xor r1, r1, r1
jal sub
daddui r1, r0, 20
halt


sub:
daddui r1, r0, 10
jr r31
