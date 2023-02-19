from enum import IntEnum
import subprocess

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


class ArgType(IntEnum):
    Immediate = 0
    Register = 1
    FRegister = 2
    ImmWReg = 3


class Instruction:
    name: str
    args: list[ArgType]

    def __init__(self, ins: str):
        if " " not in ins:
            self.name = ins
            self.args = []
        else:
            name, args = ins.split(" ", 1)
            args = args.split(",")
            self.name = name
            self.args = []
            for arg in args:
                if "(" in arg:
                    self.args.append(ArgType.ImmWReg)
                elif "imm" in arg:
                    self.args.append(ArgType.Immediate)
                elif "freg" in arg:
                    self.args.append(ArgType.FRegister)
                elif "reg" in arg:
                    self.args.append(ArgType.Register)

    def _arg_to_str(self, arg: ArgType):
        if arg == ArgType.Immediate:
            return "1"
        elif arg == ArgType.Register:
            return "r1"
        elif arg == ArgType.FRegister:
            return "f1"
        elif arg == ArgType.ImmWReg:
            return "1(r1)"
        else:
            raise ValueError(f"Invalid arg type: {arg}")

    def _arg_for_array(self, arg: ArgType):
        if arg == ArgType.Immediate:
            return "ArgumentType::Imm"
        elif arg == ArgType.Register:
            return "ArgumentType::Reg"
        elif arg == ArgType.FRegister:
            return "ArgumentType::Freg"
        elif arg == ArgType.ImmWReg:
            return "ArgumentType::ImmWReg"

    def for_array(self):
        return f"Array<ArgumentType, 3>{{{', '.join([self._arg_for_array(arg) for arg in self.args])}}}"

    def __repr__(self):
        return f"Instruction({self.name}, {self.args})"

    def __str__(self):
        if len(self.args) > 0:
            return (
                f"{self.name} {', '.join([self._arg_to_str(arg) for arg in self.args])}"
            )
        else:
            return self.name


insn = [Instruction(x.strip()) for x in insns.strip().split("\n")]

with open("all_instructions.s", "w") as f:
    f.write(".text\n")
    for ins in insn:
        f.write(str(ins) + "\n")

process = subprocess.run(
    [".\\build\\ASQMips\\ASQMips.exe", "-d", ".\\all_instructions.s"],
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
)

if process.returncode != 0:
    raise ValueError("Failed to assemble")

failed = False
with open("expected.cod", "r") as f, open("instructions.cod", "r") as g:
    instructions = [str(ins) for ins in insn]
    og_codes = [int(l, 16) for l in f.readlines()]
    my_codes = [int(l, 16) for l in g.readlines()]
    for ins, og_code, my_code in zip(instructions, og_codes, my_codes):
        if og_code != my_code:
            print(
                f"Instruction {ins} failed the check, expected {og_code} but got {my_code}"
            )
            failed = True
if not failed:
    print("All instructions passed the check!")
