#include "InstructionParser.h"
#include "CPU.h"

#include <File.h>
#include <Printer.h>

#define TODO_INS(x) ASSERT_NOT_REACHED("TODO INSTRUCTION: " #x "\n")

InstructionData::InstructionData(const Path& p) {
    auto file = File{p};
    file.open(OpenFileMode::Read);
    auto lines_or_error = file.read_all();
    if (lines_or_error.is_error()) { return; }
    auto lines = lines_or_error.to_ok();
    for (const auto& line : lines.split("\n").view().filter([](String& line) {
             line.itrim();
             return !line.is_empty();
         })) {
        instructions.append(Instruction{StrViewToUInt(line.view(), 16)});
    }
}
MAKE_FANCY_ENUM(InsType, Reg, Imm, Fp, SMTC1, SMFC1, SBC1T, SBC1F);
MAKE_FANCY_ENUM(FPIns, ADD_D = 0, SUB_D = 1, MUL_D = 2, DIV_D = 3, MOV_D = 6, CVT_D_L = 33, CVT_L_D = 37, C_LT_D = 60,
                C_LE_D = 62, C_EQ_D = 50);
MAKE_FANCY_ENUM(RegIns, NOP = 0, JR = 8, JALR = 9, MOVZ = 10, MOVN = 11, DSLLV = 20, DSRLV = 22, DSRAV = 23, DMUL = 28,
                DMULU = 29, DDIV = 30, DDIVU = 31, AND = 36, OR = 37, XOR = 38, SLT = 42, SLTU = 43, DADD = 44,
                DADDU = 45, DSUB = 46, DSUBU = 47, DSLL = 56, DSRL = 58, DSRA = 59);
MAKE_FANCY_ENUM(ImmIns, HALT = 1, J = 2, JAL = 3, BEQ = 4, BNE = 5, BEQZ = 6, BNEZ = 7, DADDI = 24, DADDIU = 25,
                SLTI = 10, SLTIU = 11, ANDI = 12, ORI = 13, XORI = 14, LUI = 15, LB = 32, LH = 33, LW = 35, LBU = 36,
                LHU = 37, LWU = 39, SB = 40, SH = 41, SW = 43, L_D = 53, S_D = 61, LD = 55, SD = 63);
Tuple<int32_t, int32_t, int32_t> extract_fp_regs_from_instruction(uint32_t opcode) {
    int32_t rs = (opcode >> 11) & 0x1F;
    int32_t rt = (opcode >> 16) & 0x1F;
    int32_t rd = (opcode >> 6) & 0x1F;
    return Tuple{rs, rt, rd};
}
Tuple<int32_t, int32_t, int16_t> extract_i_instruction(uint32_t opcode) {
    int32_t rs = (opcode >> 21) & 0x1F;
    int32_t rt = (opcode >> 16) & 0x1F;
    int16_t w = static_cast<int16_t>(opcode & 0xffff);
    return Tuple{rs, rt, w};
}
Tuple<int32_t, int32_t, int32_t> extract_r_instruction(uint32_t opcode) {
    int32_t rs = (opcode >> 21) & 0x1F;
    int32_t rt = (opcode >> 16) & 0x1F;
    int32_t rd = (opcode >> 11) & 0x1F;
    return Tuple{rs, rt, rd};
}
int32_t extract_j_instruction(uint32_t opcode, const CPU& cpu) {
    int32_t w = opcode & 0x3ffffff;
    w *= 4;
    return w;
}
Pair<int32_t, int32_t> extract_m_instruction(uint32_t opcode) {
    int32_t rt = (opcode >> 16) & 0x1F;
    int32_t rd = (opcode >> 11) & 0x1F;
    return Pair{rt, rd};
}
int16_t extract_b_instruction(uint32_t opcode, const CPU& cpu) {
    int16_t w = static_cast<int16_t>(opcode & 0xffff);
    w *= 4;
    return w;
}
static void decode_immediate_impl(uint32_t opcode, ImmIns ins, CPU& cpu) {
    switch (ins) {
    case ImmIns::HALT: {
        cpu.halt();
        Printer::print("halt");
    } break;
    case ImmIns::J: {
        auto w = extract_j_instruction(opcode, cpu);
        Printer::print("j {}", w);
        TODO_INS(J)
    } break;
    case ImmIns::JAL: {
        auto w = extract_j_instruction(opcode, cpu);
        Printer::print("jal {}", w);
        TODO_INS(JAL)
    } break;
    case ImmIns::BEQ: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        Printer::print("beq r{}, r{}, {}", rt, rs, w);
        TODO_INS(BEQ)
    } break;
    case ImmIns::BNE: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        Printer::print("bne r{}, r{}, {}", rt, rs, w);
        TODO_INS(BNE)
    } break;
    case ImmIns::BEQZ: {
        auto [_, rt, w] = extract_i_instruction(opcode);
        w *= 4;
        if (cpu.reg(rt) == 0) cpu.move_pc(w);
        Printer::print("beqz r{}, {}", rt, w);
    } break;
    case ImmIns::BNEZ: {
        auto [_, rt, w] = extract_i_instruction(opcode);
        w *= 4;
        if (cpu.reg(rt) != 0) cpu.move_pc(w);
        Printer::print("bnez r{}, {}", rt, w);
    } break;
    case ImmIns::DADDI: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.reg(rt, cpu.reg(rs) + static_cast<uint64_t>(w));
        Printer::print("daddi r{}, r{}, {}", rt, rs, w);
    } break;
    case ImmIns::DADDIU: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.reg(rt, cpu.reg(rs) + static_cast<uint64_t>(w));
        Printer::print("daddiu r{}, r{}, {}", rt, rs, w);
    } break;
    case ImmIns::SLTI: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        Printer::print("slti r{}, r{}, {}", rt, rs, w);
        TODO_INS(SLTI)
    } break;
    case ImmIns::SLTIU: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        Printer::print("sltiu r{}, r{}, {}", rt, rs, w);
        TODO_INS(SLTIU)
    } break;
    case ImmIns::ANDI: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.reg(rt, cpu.reg(rs) & static_cast<uint64_t>(w));
        Printer::print("andi r{}, r{}, {}", rt, rs, w);
    } break;
    case ImmIns::ORI: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.reg(rt, cpu.reg(rs) | static_cast<uint64_t>(w));
        Printer::print("ori r{}, r{}, {}", rt, rs, w);
    } break;
    case ImmIns::XORI: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.reg(rt, cpu.reg(rs) ^ static_cast<uint64_t>(w));
        Printer::print("xori r{}, r{}, {}", rt, rs, w);
    } break;
    case ImmIns::LUI: {
        auto [_, rt, w] = extract_i_instruction(opcode);
        Printer::print("lui r{}, {}", rt, w);
        TODO_INS(LUI)
    } break;
    case ImmIns::LB: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.reg(rt, cpu.read<1>(cpu.reg(rs) + static_cast<uint64_t>(w)));
        Printer::print("lb r{}, {}(r{})", rt, w, rs);
    } break;
    case ImmIns::LH: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        Printer::print("lh r{}, {}(r{})", rt, w, rs);
        TODO_INS(LH)
    } break;
    case ImmIns::LW: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        Printer::print("lw r{}, {}(r{})", rt, w, rs);
        TODO_INS(LW)
    } break;
    case ImmIns::LBU: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        Printer::print("lbu r{}, {}(r{})", rt, w, rs);
        TODO_INS(LBU)
    } break;
    case ImmIns::LHU: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        Printer::print("lhu r{}, {}(r{})", rt, w, rs);
        TODO_INS(LHU)
    } break;
    case ImmIns::LWU: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        Printer::print("lwu r{}, {}(r{})", rt, w, rs);
        TODO_INS(LWU)
    } break;
    case ImmIns::SB: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        Printer::print("sb r{}, {}(r{})", rt, w, rs);
        TODO_INS(SB)
    } break;
    case ImmIns::SH: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        Printer::print("sh r{}, {}(r{})", rt, w, rs);
        TODO_INS(SH)
    } break;
    case ImmIns::SW: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        Printer::print("sw r{}, {}(r{})", rt, w, rs);
        TODO_INS(SW)
    } break;
    case ImmIns::L_D: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.freg(rt, cpu.readf<8>(cpu.reg(rs) + static_cast<uint64_t>(w)));
        Printer::print("l.d f{}, {}(r{})", rt, w, rs);
    } break;
    case ImmIns::S_D: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        Printer::print("s.d f{}, {}(r{})", rt, w, rs);
        TODO_INS(S_D)
    } break;
    case ImmIns::LD: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        Printer::print("ld r{}, {}(r{})", rt, w, rs);
        TODO_INS(LD)
    } break;
    case ImmIns::SD: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        Printer::print("sd r{}, {}(r{})", rt, w, rs);
        TODO_INS(SD)
    } break;
    }
}
static void decode_register_impl(uint32_t opcode, RegIns ins, CPU& cpu) {
    switch (ins) {
    case RegIns::NOP: {
        Printer::print("nop");
        // DONE
    } break;
    case RegIns::JR: {
        auto [_, rt, __] = extract_r_instruction(opcode);
        Printer::print("jr r{}", rt);
        TODO_INS(JR)
    } break;
    case RegIns::JALR: {
        auto [_, rt, __] = extract_r_instruction(opcode);
        Printer::print("jalr r{}", rt);
        TODO_INS(JALR)
    } break;
    case RegIns::MOVZ: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        Printer::print("movz r{}, r{}, r{}", rd, rs, rt);
        TODO_INS(MOVZ)
    } break;
    case RegIns::MOVN: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        Printer::print("movn r{}, r{}, r{}", rd, rs, rt);
        TODO_INS(MOVN)
    } break;
    case RegIns::DSLLV: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        Printer::print("dsllv r{}, r{}, r{}", rd, rs, rt);
        TODO_INS(DSLLV)
    } break;
    case RegIns::DSRLV: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        Printer::print("dsrlv r{}, r{}, r{}", rd, rs, rt);
        TODO_INS(DSRLV)
    } break;
    case RegIns::DSRAV: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        Printer::print("dsrav r{}, r{}, r{}", rd, rs, rt);
        TODO_INS(DSRAV)
    } break;
    case RegIns::DMUL: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        Printer::print("dmul r{}, r{}, r{}", rd, rs, rt);
        TODO_INS(DMUL)
    } break;
    case RegIns::DMULU: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        Printer::print("dmulu r{}, r{}, r{}", rd, rs, rt);
        TODO_INS(DMULU)
    } break;
    case RegIns::DDIV: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        Printer::print("ddiv r{}, r{}, r{}", rd, rs, rt);
        TODO_INS(DDIV)
    } break;
    case RegIns::DDIVU: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        Printer::print("ddivu r{}, r{}, r{}", rd, rs, rt);
        TODO_INS(DDIVU)
    } break;
    case RegIns::AND: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        Printer::print("and r{}, r{}, r{}", rd, rs, rt);
        TODO_INS(AND)
    } break;
    case RegIns::OR: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        Printer::print("or r{}, r{}, r{}", rd, rs, rt);
        TODO_INS(OR)
    } break;
    case RegIns::XOR: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        Printer::print("xor r{}, r{}, r{}", rd, rs, rt);
        TODO_INS(XOR)
    } break;
    case RegIns::SLT: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        cpu.reg(rd, cpu.reg(rs) < cpu.reg(rt));
        Printer::print("slt r{}, r{}, r{}", rd, rs, rt);
    } break;
    case RegIns::SLTU: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        Printer::print("sltu r{}, r{}, r{}", rd, rs, rt);
        TODO_INS(SLTU)
    } break;
    case RegIns::DADD: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        Printer::print("dadd r{}, r{}, r{}", rd, rs, rt);
        TODO_INS(DADD)
    } break;
    case RegIns::DADDU: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        Printer::print("daddu r{}, r{}, r{}", rd, rs, rt);
        TODO_INS(DADDU)
    } break;
    case RegIns::DSUB: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        Printer::print("dsub r{}, r{}, r{}", rd, rs, rt);
        TODO_INS(DSUB)
    } break;
    case RegIns::DSUBU: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        Printer::print("subu r{}, r{}, r{}", rd, rs, rt);
        TODO_INS(DSUBU)
    } break;
    case RegIns::DSLL: {
        auto [rs, _, rd] = extract_r_instruction(opcode);
        auto flags = (opcode >> 6) & 0b11111;
        Printer::print("dsll r{}, r{}, {}", rd, rs, flags);
        TODO_INS(DSLL)
    } break;
    case RegIns::DSRL: {
        auto [rs, _, rd] = extract_r_instruction(opcode);
        auto flags = (opcode >> 6) & 0b11111;
        Printer::print("dsrl r{}, r{}, {}", rd, rs, flags);
        TODO_INS(DSRL)
    } break;
    case RegIns::DSRA: {
        auto [rs, _, rd] = extract_r_instruction(opcode);
        auto flags = (opcode >> 6) & 0b11111;
        Printer::print("dsra r{}, r{}, {}", rd, rs, flags);
        TODO_INS(DSRA)
    } break;
    }
}
static void decode_fp_impl(uint32_t opcode, FPIns ins) {
    auto [rs, rt, rd] = extract_fp_regs_from_instruction(opcode);
    switch (ins) {
    case FPIns::ADD_D: {
        Printer::print("add.d f{}, f{}, f{}", rs, rt, rd);
        TODO_INS(ADD_D)
    } break;
    case FPIns::SUB_D: {
        Printer::print("sub.d f{}, f{}, f{}", rs, rt, rd);
        TODO_INS(SUB_D)
    } break;
    case FPIns::MUL_D: {
        Printer::print("mul.d f{}, f{}, f{}", rs, rt, rd);
        TODO_INS(MUL_D)
    } break;
    case FPIns::DIV_D: {
        Printer::print("div.d f{}, f{}, f{}", rs, rt, rd);
        TODO_INS(DIV_D)
    } break;
    case FPIns::MOV_D: {
        Printer::print("mov.d f{}, f{}", rd, rs);
        TODO_INS(MOV_D)
    } break;
    case FPIns::CVT_D_L: {
        Printer::print("cvt.d.l f{}, f{}", rd, rs);
        TODO_INS(CVT_D_L)
    } break;
    case FPIns::CVT_L_D: {
        Printer::print("cvt.l.d f{}, f{}", rd, rs);
        TODO_INS(CVT_L_D)
    } break;
    case FPIns::C_LT_D: {
        Printer::print("c.lt.d f{}, f{}", rs, rt);
        TODO_INS(C_LT_D)
    } break;
    case FPIns::C_LE_D: {
        Printer::print("c.le.d f{}, f{}", rs, rt);
        TODO_INS(C_LE_D)
    } break;
    case FPIns::C_EQ_D: {
        Printer::print("c.eq.d f{}, f{}", rs, rt);
        TODO_INS(C_EQ_D)
    } break;
    }
}

static void decode_impl(uint32_t opcode, InsType type, uint32_t num, CPU& cpu) {
    switch (type) {
    case InsType::Imm: {
        ImmIns ins = static_cast<ImmIns>(num);
        decode_immediate_impl(opcode, ins, cpu);
    } break;
    case InsType::Reg: {
        RegIns ins = static_cast<RegIns>(num);
        decode_register_impl(opcode, ins, cpu);
    } break;
    case InsType::Fp: {
        FPIns ins = static_cast<FPIns>(num);
        decode_fp_impl(opcode, ins);
    } break;
    case InsType::SMTC1: {
        // move data from integer register to FP register
        auto [rt, rd] = extract_m_instruction(opcode);
        Printer::print("mtc1 r{}, f{}", rt, rd);
        TODO_INS(SMTC1)
    } break;
    case InsType::SMFC1: {
        // move data from FP register to integer register
        auto [rt, rd] = extract_m_instruction(opcode);
        Printer::print("mfc1 r{}, f{}", rt, rd);
        TODO_INS(SMFC1)
    } break;
    case InsType::SBC1T: {
        int16_t w = extract_b_instruction(opcode, cpu);
        Printer::print("bc1t {}", w);
        TODO_INS(SBC1T)
    } break;
    case InsType::SBC1F: {
        int16_t w = extract_b_instruction(opcode, cpu);
        Printer::print("bc1t {}", w);
        TODO_INS(SBC1F)
    } break;
    }
}
void Instruction::decode(CPU& cpu) {
    uint32_t instruction_id = (opcode >> 26) & 0b111111;
    uint32_t bits_check = (opcode >> 21) & 0b11111;
    if (instruction_id == 0) {
        // I_SPECIAL (SR)
        instruction_id = opcode & 0b111111;
        decode_impl(opcode, InsType::Reg, instruction_id, cpu);
    } else if (instruction_id == 0x11 && bits_check == 0x11) {
        // I_COP1 + I_DOUBLE (SF)
        instruction_id = opcode & 0b111111;
        // floating point instructions
        decode_impl(opcode, InsType::Fp, instruction_id, cpu);
    } else if (instruction_id == 0x11) {
        // special cases
        if (bits_check == 0x04) {
            // SMTC1
            decode_impl(opcode, InsType::SMTC1, instruction_id, cpu);
        } else if (bits_check == 0x08 && (opcode & (1 << 16))) {
            // SBC1T
            decode_impl(opcode, InsType::SBC1T, instruction_id, cpu);
        } else if (bits_check == 0x08) {
            // SBC1F
            decode_impl(opcode, InsType::SBC1F, instruction_id, cpu);
        } else {
            // SMFC1
            decode_impl(opcode, InsType::SMFC1, instruction_id, cpu);
        }
    } else {
        // SI instructions
        decode_impl(opcode, InsType::Imm, instruction_id, cpu);
    }
}