.text
daddui r10, r0, 0

loop:
daddui r10, r10, 1
daddi r11, r10, -99
bnez r11, loop
halt