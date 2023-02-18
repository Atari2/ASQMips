insns = """
lb reg,imm(reg)     
lbu reg,imm(reg)    
sb reg,imm(reg)     
lh reg,imm(reg)     
lhu reg,imm(reg)    
sh reg,imm(reg)     
lw reg,imm(reg)     
lwu reg,imm(reg)    
sw reg,imm(reg)     
ld reg,imm(reg)     
sd reg,imm(reg)     
l.d freg,imm(reg)   
s.d freg,imm(reg)   
halt                
daddi reg,reg,imm   
daddui reg,reg,imm  
andi reg,reg,imm    
ori reg,reg,imm     
xori reg,reg,imm    
lui  reg,imm        
slti reg,reg,imm    
sltiu reg,reg,imm   
beq reg,reg,imm     
bne reg,reg,imm     
beqz reg,imm        
bnez reg,imm        
j imm               
jr reg              
jal imm              
jalr reg             
dsll reg,reg,imm     
dsrl reg,reg,imm     
dsra reg,reg,imm     
dsllv reg,reg,reg    
dsrlv reg,reg,reg    
dsrav reg,reg,reg    
movz reg,reg,reg     
movn reg,reg,reg     
nop                  
and reg,reg,reg      
or reg,reg,reg       
xor reg,reg,reg      
slt reg,reg,reg      
sltu reg,reg,reg     
dadd reg,reg,reg     
daddu reg,reg,reg    
dsub reg,reg,reg     
dsubu reg,reg,reg    
dmul reg,reg,reg     
dmulu reg,reg,reg    
ddiv reg,reg,reg     
ddivu reg,reg,reg    
add.d freg,freg,freg 
sub.d freg,freg,freg 
mul.d freg,freg,freg 
div.d freg,freg,freg 
mov.d freg,freg      
cvt.d.l freg,freg    
cvt.l.d freg,freg    
c.lt.d freg,freg     
c.le.d freg,freg     
c.eq.d freg,freg     
bc1f imm             
bc1t imm             
mtc1 reg,freg       
mfc1 reg,freg       
"""
insn = [x.strip() for x in insns.strip().split("\n")]
for ins in insn:
    if " " not in ins:
        print("{},")
    else:
        args = ins.split(" ", 1)[-1].split(",")
        argstr = "{"
        for arg in args:
            if "(" in arg:
                argstr += "ArgumentType::ImmWReg, "
            elif "imm" in arg:
                argstr += "ArgumentType::Imm, "
            elif "reg" in arg:
                argstr += "ArgumentType::Reg, "
        argstr = argstr.strip(", ") + "}"
        print(f"{argstr}, ")
