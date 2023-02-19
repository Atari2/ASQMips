#include "InstructionParser.h"
#include "Parser.h"

enum class OpcodeType : uint8_t { R = 1, I = 2, J = 3, F = 4, M = 5, B = 6 };

enum class SubType : uint8_t {
    NOP = 0,
    LOAD = 1,
    STORE = 2,
    REG1I = 3,
    REG2I = 4,
    REG2S = 5,
    JUMP = 6,
    JREG = 7,
    HALT = 8,
    REG3F = 9,
    BRANCH = 10,
    REG3 = 11,
    REGID = 12,
    FLOAD = 13,
    FSTORE = 14,
    JREGN = 15,
    REG2F = 16,
    REG3X = 17,
    REGDI = 18,
    REG2C = 19,
    BC = 20
};
struct OpcodeInfo {
    OpcodeType type;
    SubType subtype;
    uint32_t op_code;
};

enum Opcode : uint32_t {
    I_SPECIAL = 0x00,
    I_COP1 = 0x11,
    I_DOUBLE = 0x11,
    I_MTC1 = 0x04,
    I_MFC1 = 0x00,
    I_BC = 0x08,
    I_HALT = 0x01,

    I_J = 0x02,
    I_JAL = 0x03,
    I_BEQ = 0x04,
    I_BNE = 0x05,
    I_BEQZ = 0x06,
    I_BNEZ = 0x07,

    I_DADDI = 0x18,
    I_DADDIU = 0x19,
    I_SLTI = 0x0A,
    I_SLTIU = 0x0B,
    I_ANDI = 0x0C,
    I_ORI = 0x0D,
    I_XORI = 0x0E,
    I_LUI = 0x0F,

    I_LB = 0x20,
    I_LH = 0x21,
    I_LW = 0x23,
    I_LBU = 0x24,
    I_LHU = 0x25,
    I_LWU = 0x27,
    I_SB = 0x28,
    I_SH = 0x29,
    I_SW = 0x2B,
    I_L_D = 0x35,
    I_S_D = 0x3D,
    I_LD = 0x37,
    I_SD = 0x3F,

    R_NOP = 0x00,
    R_JR = 0x08,
    R_JALR = 0x09,
    R_MOVZ = 0x0A,
    R_MOVN = 0x0B,

    R_DSLLV = 0x14,
    R_DSRLV = 0x16,
    R_DSRAV = 0x17,
    R_DMUL = 0x1C,
    R_DMULU = 0x1D,
    R_DDIV = 0x1E,
    R_DDIVU = 0x1F,

    R_AND = 0x24,
    R_OR = 0x25,
    R_XOR = 0x26,
    R_SLT = 0x2A,
    R_SLTU = 0x2B,
    R_DADD = 0x2C,
    R_DADDU = 0x2D,
    R_DSUB = 0x2E,
    R_DSUBU = 0x2F,

    R_DSLL = 0x38,
    R_DSRL = 0x3A,
    R_DSRA = 0x3B,

    F_ADD_D = 0x00,
    F_SUB_D = 0x01,
    F_MUL_D = 0x02,
    F_DIV_D = 0x03,
    F_MOV_D = 0x06,
    F_CVT_D_L = 0x21,
    F_CVT_L_D = 0x25,
    F_C_LT_D = 0x3C,
    F_C_LE_D = 0x3E,
    F_C_EQ_D = 0x32,
};

auto SI = [](Opcode x) -> Opcode {
    return UpCast<Opcode>(x << 26);
};
auto SR = [](Opcode x) -> Opcode {
    return UpCast<Opcode>(x | Opcode::I_SPECIAL << 26);
};
auto SF = [](Opcode x) -> Opcode {
    return UpCast<Opcode>(x | Opcode::I_COP1 << 26 | Opcode::I_DOUBLE << 21);
};
auto SMTC1 = []() -> Opcode {
    return UpCast<Opcode>((Opcode::I_COP1 << 26 | Opcode::I_MTC1 << 21));
};
auto SMFC1 = []() -> Opcode {
    return UpCast<Opcode>((Opcode::I_COP1 << 26 | Opcode::I_MFC1 << 21));
};
auto SBC1F = []() -> Opcode {
    return UpCast<Opcode>((Opcode::I_COP1 << 26 | Opcode::I_BC << 21));
};
auto SBC1T = []() -> Opcode {
    return UpCast<Opcode>((Opcode::I_COP1 << 26 | Opcode::I_BC << 21 | 1 << 16));
};

constexpr static OpcodeInfo codes[] = {{OpcodeType::I, SubType::LOAD, SI(Opcode::I_LB)},
                                       {OpcodeType::I, SubType::LOAD, SI(Opcode::I_LBU)},
                                       {OpcodeType::I, SubType::STORE, SI(Opcode::I_SB)},
                                       {OpcodeType::I, SubType::LOAD, SI(Opcode::I_LH)},
                                       {OpcodeType::I, SubType::LOAD, SI(Opcode::I_LHU)},
                                       {OpcodeType::I, SubType::STORE, SI(Opcode::I_SH)},
                                       {OpcodeType::I, SubType::LOAD, SI(Opcode::I_LW)},
                                       {OpcodeType::I, SubType::LOAD, SI(Opcode::I_LWU)},
                                       {OpcodeType::I, SubType::STORE, SI(Opcode::I_SW)},
                                       {OpcodeType::I, SubType::LOAD, SI(Opcode::I_LD)},
                                       {OpcodeType::I, SubType::STORE, SI(Opcode::I_SD)},
                                       {OpcodeType::I, SubType::FLOAD, SI(Opcode::I_L_D)},
                                       {OpcodeType::I, SubType::FSTORE, SI(Opcode::I_S_D)},
                                       {OpcodeType::I, SubType::HALT, SI(Opcode::I_HALT)},

                                       {OpcodeType::I, SubType::REG2I, SI(Opcode::I_DADDI)},
                                       {OpcodeType::I, SubType::REG2I, SI(Opcode::I_DADDIU)},
                                       {OpcodeType::I, SubType::REG2I, SI(Opcode::I_ANDI)},
                                       {OpcodeType::I, SubType::REG2I, SI(Opcode::I_ORI)},
                                       {OpcodeType::I, SubType::REG2I, SI(Opcode::I_XORI)},
                                       {OpcodeType::I, SubType::REG1I, SI(Opcode::I_LUI)},

                                       {OpcodeType::I, SubType::REG2I, SI(Opcode::I_SLTI)},
                                       {OpcodeType::I, SubType::REG2I, SI(Opcode::I_SLTIU)},

                                       {OpcodeType::I, SubType::BRANCH, SI(Opcode::I_BEQ)},
                                       {OpcodeType::I, SubType::BRANCH, SI(Opcode::I_BNE)},
                                       {OpcodeType::I, SubType::JREGN, SI(Opcode::I_BEQZ)},
                                       {OpcodeType::I, SubType::JREGN, SI(Opcode::I_BNEZ)},

                                       {OpcodeType::J, SubType::JUMP, SI(Opcode::I_J)},
                                       {OpcodeType::R, SubType::JREG, SR(Opcode::R_JR)},
                                       {OpcodeType::J, SubType::JUMP, SI(Opcode::I_JAL)},
                                       {OpcodeType::R, SubType::JREG, SR(Opcode::R_JALR)},

                                       {OpcodeType::R, SubType::REG2S, SR(Opcode::R_DSLL)},
                                       {OpcodeType::R, SubType::REG2S, SR(Opcode::R_DSRL)},
                                       {OpcodeType::R, SubType::REG2S, SR(Opcode::R_DSRA)},
                                       {OpcodeType::R, SubType::REG3, SR(Opcode::R_DSLLV)},
                                       {OpcodeType::R, SubType::REG3, SR(Opcode::R_DSRLV)},
                                       {OpcodeType::R, SubType::REG3, SR(Opcode::R_DSRAV)},
                                       {OpcodeType::R, SubType::REG3, SR(Opcode::R_MOVZ)},
                                       {OpcodeType::R, SubType::REG3, SR(Opcode::R_MOVN)},
                                       {OpcodeType::R, SubType::NOP, SR(Opcode::R_NOP)},
                                       {OpcodeType::R, SubType::REG3, SR(Opcode::R_AND)},
                                       {OpcodeType::R, SubType::REG3, SR(Opcode::R_OR)},
                                       {OpcodeType::R, SubType::REG3, SR(Opcode::R_XOR)},
                                       {OpcodeType::R, SubType::REG3, SR(Opcode::R_SLT)},
                                       {OpcodeType::R, SubType::REG3, SR(Opcode::R_SLTU)},

                                       {OpcodeType::R, SubType::REG3, SR(Opcode::R_DADD)},
                                       {OpcodeType::R, SubType::REG3, SR(Opcode::R_DADDU)},
                                       {OpcodeType::R, SubType::REG3, SR(Opcode::R_DSUB)},
                                       {OpcodeType::R, SubType::REG3, SR(Opcode::R_DSUBU)},

                                       {OpcodeType::R, SubType::REG3, SR(Opcode::R_DMUL)},
                                       {OpcodeType::R, SubType::REG3, SR(Opcode::R_DMULU)},
                                       {OpcodeType::R, SubType::REG3, SR(Opcode::R_DDIV)},
                                       {OpcodeType::R, SubType::REG3, SR(Opcode::R_DDIVU)},

                                       {OpcodeType::F, SubType::REG3F, SF(Opcode::F_ADD_D)},
                                       {OpcodeType::F, SubType::REG3F, SF(Opcode::F_SUB_D)},
                                       {OpcodeType::F, SubType::REG3F, SF(Opcode::F_MUL_D)},
                                       {OpcodeType::F, SubType::REG3F, SF(Opcode::F_DIV_D)},
                                       {OpcodeType::F, SubType::REG2F, SF(Opcode::F_MOV_D)},
                                       {OpcodeType::F, SubType::REG2F, SF(Opcode::F_CVT_D_L)},
                                       {OpcodeType::F, SubType::REG2F, SF(Opcode::F_CVT_L_D)},
                                       {OpcodeType::F, SubType::REG2C, SF(Opcode::F_C_LT_D)},
                                       {OpcodeType::F, SubType::REG2C, SF(Opcode::F_C_LE_D)},
                                       {OpcodeType::F, SubType::REG2C, SF(Opcode::F_C_EQ_D)},

                                       {OpcodeType::B, SubType::BC, SBC1F()},
                                       {OpcodeType::B, SubType::BC, SBC1T()},
                                       {OpcodeType::M, SubType::REGID, SMTC1()},
                                       {OpcodeType::M, SubType::REGDI, SMFC1()}};

uint32_t InstructionData::encode() const {
    auto op_info = codes[ToUnderlying(info->insn)];
    uint32_t rs = 0;
    uint32_t rt = 0;
    uint32_t rd = 0;
    uint32_t w = 0;
    uint32_t flags = 0;

    uint32_t opcode = 0;

    switch (info->insn) {
    // LOAD/STORE
    case Instruction::LoadByte:
    case Instruction::LoadByteUnsigned:
    case Instruction::StoreByte:
    case Instruction::LoadHalfWord:
    case Instruction::LoadHalfWordUnsigned:
    case Instruction::StoreHalfWord:
    case Instruction::LoadWord:
    case Instruction::LoadWordUnsigned:
    case Instruction::StoreWord:
    case Instruction::LoadDoubleWord:
    case Instruction::StoreDoubleWord: {
        rt = ToUnderlying(args[0].m_reg());
        if (info->arg_types[1] == ArgumentType::ImmWReg) {
            w = args[1].m_imm_reg().first().get<int32_t>();
            rs = ToUnderlying(args[1].m_imm_reg().second());
        } else {
            w = 0;
            rs = ToUnderlying(args[1].m_reg());
        }
    } break;
    // FSTORE/FLOAD
    case Instruction::LoadReal:
    case Instruction::StoreReal: {
        rt = ToUnderlying(args[0].m_reg()) - ToUnderlying(RegisterEnum::f0);
        if (info->arg_types[1] == ArgumentType::ImmWReg) {
            w = args[1].m_imm_reg().first().get<int32_t>();
            rs = ToUnderlying(args[1].m_imm_reg().second());
        } else {
            w = 0;
            rs = ToUnderlying(args[1].m_reg());
        }
    } break;
    // NOP + HALT
    case Instruction::Nop:
    case Instruction::Halt:
        break;
    // REG2I
    case Instruction::AddImmediate:
    case Instruction::AddImmediateUnsigned:
    case Instruction::LogicalAndImmediate:
    case Instruction::LogicalOrImmediate:
    case Instruction::LogicalXorImmediate:
    case Instruction::SetLessThanImmediate:
    case Instruction::SetLessThanImmediateUnsigned: {
        rt = ToUnderlying(args[0].m_reg());
        rs = ToUnderlying(args[1].m_reg());
        w = args[2].m_imm().get<int32_t>();
    } break;
    // REG1I
    case Instruction::LoadUpperImmediate: {
        rt = ToUnderlying(args[0].m_reg());
        w = args[1].m_imm().get<int32_t>();
    } break;
    // BRANCH
    case Instruction::BranchIfEqual:
    case Instruction::BranchIfNotEqual: {
        rt = ToUnderlying(args[0].m_reg());
        rs = ToUnderlying(args[1].m_reg());
        auto rel = args[2].m_imm().get<int32_t>();
        rel -= pc_address + 4;
        rel /= 4;
        w = rel;
    } break;
    // JREGN
    case Instruction::BranchIfZero:
    case Instruction::BranchIfNotZero: {
        rt = ToUnderlying(args[0].m_reg());
        auto rel = args[1].m_imm().get<int32_t>();
        rel -= pc_address + 4;
        rel /= 4;
        w = rel;
    } break;
    // JUMP+BC
    case Instruction::Jump:
    case Instruction::JumpAndLink:
    case Instruction::BranchIfFpFlagNotSet:
    case Instruction::BranchIfFpFlagSet: {
        auto rel = args[0].m_imm().get<int32_t>();
        rel -= pc_address + 4;
        rel /= 4;
        w = rel;
    } break;
    // JREG
    case Instruction::JumpToReg:
    case Instruction::JumpAndLinkToReg: {
        rt = ToUnderlying(args[0].m_reg());
    } break;
    // REG2S
    case Instruction::ShiftLefLogical:
    case Instruction::ShiftRightLogical:
    case Instruction::ShiftRightArithmetic: {
        rd = ToUnderlying(args[0].m_reg());
        rs = ToUnderlying(args[1].m_reg());
        flags = args[2].m_imm().get<int32_t>();
    } break;
    // REG3
    case Instruction::ShiftLeftByVar:
    case Instruction::ShiftRightByVar:
    case Instruction::ShiftRightArithByVar:
    case Instruction::MoveIfZero:
    case Instruction::MoveIfNotZero:
    case Instruction::LogicalAnd:
    case Instruction::LogicalOr:
    case Instruction::LogicalXor:
    case Instruction::SetLessThan:
    case Instruction::SetLessThanUnsigned:
    case Instruction::Add:
    case Instruction::AddUnsigned:
    case Instruction::Subtract:
    case Instruction::SubtractUnsigned:
    case Instruction::Multiply:
    case Instruction::MultiplyUnsigned:
    case Instruction::Divide:
    case Instruction::DivideUnsigned: {
        rd = ToUnderlying(args[0].m_reg());
        rs = ToUnderlying(args[1].m_reg());
        rt = ToUnderlying(args[2].m_reg());
    } break;
    // REG3F
    case Instruction::AddReal:
    case Instruction::SubtractReal:
    case Instruction::MultiplyReal:
    case Instruction::DivideReal: {
        rd = ToUnderlying(args[0].m_reg()) - ToUnderlying(RegisterEnum::f0);
        rs = ToUnderlying(args[1].m_reg()) - ToUnderlying(RegisterEnum::f0);
        rt = ToUnderlying(args[2].m_reg()) - ToUnderlying(RegisterEnum::f0);
    } break;
    // REG2F
    case Instruction::MoveReal:
    case Instruction::ConvertIntegerToReal:
    case Instruction::ConvertRealToInteger: {
        rd = ToUnderlying(args[0].m_reg()) - ToUnderlying(RegisterEnum::f0);
        rs = ToUnderlying(args[1].m_reg()) - ToUnderlying(RegisterEnum::f0);
    } break;
    // REG2C
    case Instruction::SetFpFlagIfLessThan:
    case Instruction::SetFpFlagIfLessThanOrEqual:
    case Instruction::SetFpFlagIfEqual: {
        rs = ToUnderlying(args[0].m_reg()) - ToUnderlying(RegisterEnum::f0);
        rt = ToUnderlying(args[1].m_reg()) - ToUnderlying(RegisterEnum::f0);
    } break;

    // REGID+REGDI
    case Instruction::MoveDataFromIntegerToFp:
    case Instruction::MoveDataFromFpToInteger: {
        rt = ToUnderlying(args[0].m_reg());
        rd = ToUnderlying(args[1].m_reg()) - ToUnderlying(RegisterEnum::f0);
    } break;
    }

    switch (op_info.type) {
    case OpcodeType::I:
        opcode = (op_info.op_code | rs << 21 | rt << 16 | (w & 0xffff));
        break;
    case OpcodeType::R:
        opcode = (op_info.op_code | rs << 21 | rt << 16 | rd << 11 | flags << 6);
        break;
    case OpcodeType::J:
        opcode = (op_info.op_code | (w & 0x3ffffff));
        break;
    case OpcodeType::F:
        opcode = (op_info.op_code | rs << 11 | rt << 16 | rd << 6);
        break;
    case OpcodeType::M:
        opcode = (op_info.op_code | rt << 16 | rd << 11);
        break;
    case OpcodeType::B:
        opcode = (op_info.op_code | (w & 0xffff));
        break;
    }
    return opcode;
}