#pragma once
#include "Tokenizer.h"
#include <Array.h>
#include <CxprHashMap.h>
#include <EnumHelpers.h>
#include <StringView.h>
#include <Tuple.h>
#include <Variant.h>

using namespace ARLib;

MAKE_FANCY_ENUM(RegisterEnum, r0, r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16, r17, r18, r19,
                r20, r21, r22, r23, r24, r25, r26, r27, r28, r29, r30, r31, f0, f1, f2, f3, f4, f5, f6, f7, f8, f9, f10,
                f11, f12, f13, f14, f15, f16, f17, f18, f19, f20, f21, f22, f23, f24, f25, f26, f27, f28, f29, f30, f31)

struct RegisterWithName {
    StringView name;
    RegisterEnum val;
    constexpr bool operator==(const RegisterWithName& other) const { return name == other.name && val == other.val; }
};

template <>
struct cxpr::Hasher<RegisterWithName> {
    constexpr uint32_t operator()(const RegisterWithName& val) const noexcept { return crc32::calculate(val.name); }
};

constexpr auto construct_register_map() {
    cxpr::HashTable<RegisterWithName, enum_size<RegisterEnum>()> table{};
    for (auto e : for_each_enum<RegisterEnum>()) {
        const auto view = enum_to_str_view(e);
        table.insert({view, e});
    }
    return table;
}

constexpr auto register_map = construct_register_map();

struct Label {
    StringView name;
    size_t address;
};

template <>
struct ARLib::PrintInfo<Label> {
    const Label& m_label;
    constexpr PrintInfo(const Label& label) noexcept : m_label(label) {}
    String repr() const {
        return "Label { "_s + m_label.name.extract_string() + " at "_s +
               IntToStr<SupportedBase::Hexadecimal, true>(m_label.address) + " }"_s;
    }
};

using Immediate = Variant<int32_t, double, StringView>;
using ImmediateWithRegister = Pair<Immediate, RegisterEnum>;

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

MAKE_FANCY_ENUM(ArgumentType, Reg, Freg, Imm, ImmWReg);

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
constexpr Array instruction_arg_info{Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::ImmWReg},
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
                                     Array<ArgumentType, 3>{ArgumentType::Freg, ArgumentType::ImmWReg},
                                     Array<ArgumentType, 3>{ArgumentType::Freg, ArgumentType::ImmWReg},
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
                                     Array<ArgumentType, 3>{ArgumentType::Freg, ArgumentType::Freg, ArgumentType::Freg},
                                     Array<ArgumentType, 3>{ArgumentType::Freg, ArgumentType::Freg, ArgumentType::Freg},
                                     Array<ArgumentType, 3>{ArgumentType::Freg, ArgumentType::Freg, ArgumentType::Freg},
                                     Array<ArgumentType, 3>{ArgumentType::Freg, ArgumentType::Freg, ArgumentType::Freg},
                                     Array<ArgumentType, 3>{ArgumentType::Freg, ArgumentType::Freg},
                                     Array<ArgumentType, 3>{ArgumentType::Freg, ArgumentType::Freg},
                                     Array<ArgumentType, 3>{ArgumentType::Freg, ArgumentType::Freg},
                                     Array<ArgumentType, 3>{ArgumentType::Freg, ArgumentType::Freg},
                                     Array<ArgumentType, 3>{ArgumentType::Freg, ArgumentType::Freg},
                                     Array<ArgumentType, 3>{ArgumentType::Freg, ArgumentType::Freg},
                                     Array<ArgumentType, 3>{ArgumentType::Imm},
                                     Array<ArgumentType, 3>{ArgumentType::Imm},
                                     Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Freg},
                                     Array<ArgumentType, 3>{ArgumentType::Reg, ArgumentType::Freg}};
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
        size_t index = static_cast<size_t>(ToUnderlying(en));
        if (en == Instruction::Nop || en == Instruction::Halt) {
            map.insert(InstructionInfo{instruction_names[index], en, 0, instruction_arg_info[index]});
        } else {
            map.insert(
            InstructionInfo{instruction_names[index], en, instruction_arg_sizes[index], instruction_arg_info[index]});
        }
    }
    return map;
}

constexpr auto instruction_map = construct_instruction_map();

struct InsnArgument {
    ArgumentType m_type;
    Variant<RegisterEnum, Immediate, ImmediateWithRegister> m_value;

    InsnArgument(ArgumentType type) : m_type{type} {
        if (type == ArgumentType::Reg || m_type == ArgumentType::Freg) {
            m_value = RegisterEnum{};
        } else if (type == ArgumentType::Imm) {
            m_value = Immediate{};
        } else if (type == ArgumentType::ImmWReg) {
            m_value = ImmediateWithRegister{Immediate{}, RegisterEnum{}};
        }
    }
    auto& m_reg() { return m_value.get<RegisterEnum>(); }
    auto& m_imm() { return m_value.get<Immediate>(); }
    auto& m_imm_reg() { return m_value.get<ImmediateWithRegister>(); }
    const auto& m_reg() const { return m_value.get<RegisterEnum>(); }
    const auto& m_imm() const { return m_value.get<Immediate>(); }
    const auto& m_imm_reg() const { return m_value.get<ImmediateWithRegister>(); }
};

struct InstructionData {
    RefBox<const InstructionInfo> info;
    Array<InsnArgument, 3> args;
    uint32_t pc_address = 0;
    InstructionData() :
        info{},
        args{InsnArgument{ArgumentType::Reg}, InsnArgument{ArgumentType::Reg}, InsnArgument{ArgumentType::Reg}} {}
    InstructionData(const InstructionInfo& inf) :
        info{inf},
        args{InsnArgument{inf.arg_types[0]}, InsnArgument{inf.arg_types[1]}, InsnArgument{inf.arg_types[2]}} {}
    InstructionData(InstructionData&& other) :
        info{other.info},
        args{InsnArgument{info->arg_types[0]}, InsnArgument{info->arg_types[1]}, InsnArgument{info->arg_types[2]}},
        pc_address{other.pc_address} {
        for (size_t idx = 0; idx < args.size(); ++idx) {
            args[idx] = move(other.args[idx]);
        };
    }
    InstructionData& operator=(InstructionData&& other) {
        info = move(other.info);
        pc_address = other.pc_address;
        for (size_t idx = 0; idx < args.size(); ++idx) {
            args[idx] = move(other.args[idx]);
        };
        return *this;
    }
    uint32_t address() const { return pc_address; }
    uint32_t encode() const;
};

template <>
struct ARLib::PrintInfo<InstructionData> {
    const InstructionData& m_data;
    constexpr PrintInfo(const InstructionData& data) : m_data(data) {}
    String repr() const {
        String str = m_data.info->name.extract_string();
        str += " "_s;
        for (size_t i = 0; i < m_data.info->arg_count; ++i) {
            if (i != 0) { str += ", "; }
            const auto& arg = m_data.args[i];
            if (arg.m_type == ArgumentType::Reg || arg.m_type == ArgumentType::Freg) {
                str += enum_to_str(arg.m_reg());
            } else if (arg.m_type == ArgumentType::Imm) {
                str += PrintInfo<Immediate>(arg.m_imm()).repr();
            } else if (arg.m_type == ArgumentType::ImmWReg) {
                str += enum_to_str(arg.m_imm_reg().second()) + "("_s +
                       PrintInfo<Immediate>(arg.m_imm_reg().first()).repr() + ")"_s;
            }
        }
        return str;
    }
};

class Parser;
tit parse_instruction(tit begin, tit end, Parser& parser);