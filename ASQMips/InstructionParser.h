#pragma once
#include "Tokenizer.h"
#include <Array.h>
#include <CxprHashMap.h>
#include <EnumHelpers.h>
#include <StringView.h>
#include <Tuple.h>

using namespace ARLib;

MAKE_FANCY_ENUM(Register, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19, r20,
                r21, r22, r23, r24, r25, r26, r27, r28, r29, r30, r31, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10, f11,
                f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29, f30, f31)

using Immediate = int32_t;
using ImmediateWithRegister = Pair<Immediate, Register>;

MAKE_FANCY_ENUM(Instruction, LoadByte, LoadByteUnsigned, StoreByte, LoadHalfWord, LoadHalfWordUnsigned, StoreHalfWord,
                LoadWord, LoadWordUnsigned, StoreWord, LoadDoubleWord, StoreDoubleWord, LoadReal, StoreReal, Halt,
                AddImmediate, AddImmediateUnsigned, LogicalAndImmediate, LogicalOrImmediate, LogicalXorImmediate,
                LoadUpperImmediate, SetLessThanImmediate, SetLessThanImmediateUnsigned, BranchIfEqual, BranchIfNotEqual,
                BranchIfZero, BranchIfNotZero, Jump, JumpToReg, JumpAndLink, JumpAndLinkToReg, ShiftLefLogical,
                ShiftRightLogical, ShiftRightArithmetic, ShiftLeftByVar, ShiftRightByVar, ShiftRightArithByVar,
                MoveIfZero, MoveIfNotZero, Nop, LogicalAnd, LogicalOr, LogicalXor, SetLessThan, SetLessThanUnsigned,
                Add, AddUnsigned, Subtract, SubtractUnsigned, Multiply, MultiplyUnsigned, Divide, DivideUnsigned,
                AddReal, SubtractReal, MultiplyReal, DivideReal, MoveReal, ConvertIntegerToReal, ConvertRealToInteger,
                SetFpFlagIfLessThan, SetFpFlagIfLessThanOrEqual, SetFpFlagIfEqual, BranchIfFpFlagNotSet,
                BranchIfFpFlagSet, MoveDataFromIntegerToFp, MoveDataFromFpToInteger)

MAKE_FANCY_ENUM(ArgumentType, Reg, Imm, ImmWReg);

constexpr Array instruction_names{
"lb"_sv,    "lbu"_sv,   "sb"_sv,    "lh"_sv,      "lhu"_sv,     "sh"_sv,     "lw"_sv,     "lwu"_sv,    "sw"_sv,
"ld"_sv,    "sd"_sv,    "l.d"_sv,   "s.d"_sv,     "halt"_sv,    "daddi"_sv,  "daddui"_sv, "andi"_sv,   "ori"_sv,
"xori"_sv,  "lui"_sv,   "slti"_sv,  "sltiu"_sv,   "beq"_sv,     "bne"_sv,    "beqz"_sv,   "bnez"_sv,   "j"_sv,
"jr"_sv,    "jal"_sv,   "jalr"_sv,  "dsll"_sv,    "dsrl"_sv,    "dsra"_sv,   "dsllv"_sv,  "dsrlv"_sv,  "dsrav"_sv,
"movz"_sv,  "movn"_sv,  "nop"_sv,   "and"_sv,     "or"_sv,      "xor"_sv,    "slt"_sv,    "sltu"_sv,   "dadd"_sv,
"daddu"_sv, "dsub"_sv,  "dsubu"_sv, "dmul"_sv,    "dmulu"_sv,   "ddiv"_sv,   "ddivu"_sv,  "add.d"_sv,  "sub.d"_sv,
"mul.d"_sv, "div.d"_sv, "mov.d"_sv, "cvt.d.l"_sv, "cvt.l.d"_sv, "c.lt.d"_sv, "c.le.d"_sv, "c.eq.d"_sv, "bc1f"_sv,
"bc1t"_sv,  "mtc1"_sv,  "mfc1"_sv};
constexpr Array<size_t, instruction_names.size()> instruction_arg_sizes{
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 2, 2, 1, 1, 1, 1, 3, 3, 3,
3, 3, 3, 3, 3, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 2, 2, 2, 2, 2, 2, 1, 1, 2, 2};
constexpr Array instruction_arg_info{
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::ImmWReg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::ImmWReg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::ImmWReg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::ImmWReg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::ImmWReg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::ImmWReg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::ImmWReg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::ImmWReg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::ImmWReg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::ImmWReg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::ImmWReg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::ImmWReg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::ImmWReg},
Array<ArgumentType, 3>{},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Imm},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Imm},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Imm},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Imm},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Imm},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Imm},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Imm},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Imm},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Imm},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Imm},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Imm},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Imm},
Array<ArgumentType, 3>{ArgumentType::Imm},
Array<ArgumentType, 3>{ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Imm},
Array<ArgumentType, 3>{ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Imm},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Imm},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Imm},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Imm},
Array<ArgumentType, 3>{ArgumentType::Imm},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg},
Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Reg},
};
static_assert(instruction_names.size() == ToUnderlying(Instruction::MoveDataFromFpToInteger) + 1);
static_assert(instruction_arg_sizes.size() == ToUnderlying(Instruction::MoveDataFromFpToInteger) + 1);
static_assert(instruction_arg_info.size() == ToUnderlying(Instruction::MoveDataFromFpToInteger) + 1);

struct InstructionInfo {
    StringView name;
    Instruction insn;
    size_t arg_count;
    Array<ArgumentType, 3> arg_types;
    constexpr bool operator==(const InstructionInfo& other) const { return name == other.name; }
};

template <>
struct cxpr::Hasher<InstructionInfo> {
    constexpr uint32_t operator()(const InstructionInfo& info) const { return cxpr::crc32::calculate(info.name); }
};

constexpr auto construct_instruction_map() {
    cxpr::HashTable<InstructionInfo, instruction_names.size()> map{};
    for (auto en : for_each_enum<Instruction>()) {
        size_t index = ToUnderlying(en);
        map.insert(
        InstructionInfo{instruction_names[index], en, instruction_arg_sizes[index], instruction_arg_info[index]});
    }
    return map;
}

constexpr auto instruction_map = construct_instruction_map();

struct InsnArgument {
    ArgumentType m_type;
    union {
        Register m_reg;
        Immediate m_imm;
        ImmediateWithRegister m_imm_reg;
    };
};

struct InstructionData {
    const InstructionInfo& info;
    Array<InsnArgument, 3> args;
    uint32_t encode() const {
        // bits 31-26 opcode
        // bits 25-21 rs
        // bits 20-16 rt
        // bits 15-0 immediate
        // TODO: implement
        return 0;
    }
};

class Parser;
tit parse_instruction(tit begin, tit end, Parser& parser);