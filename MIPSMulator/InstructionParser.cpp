#include "InstructionParser.h"
#include "CPU.h"
#include <Printer.hpp>

DiscardResult<FileError> InstructionData::load(const Path& p) {
    auto lines_or_error = File::read_all(p);
    if (lines_or_error.is_error()) { return FileError{lines_or_error.to_error()}; }
    auto lines = lines_or_error.to_ok();
    for (const auto& line :
         lines.split("\n").iter().inplace_transform([](String& line) { return line.trim(); }).filter([](String& line) {
             return !line.is_empty();
         })) {
        TRY_SET(val, StrViewToUInt(line.view(), 16));
        instructions.append(Instruction{val});
    }
    return {};
}
MAKE_FANCY_ENUM(InsType, uint8_t, Reg, Imm, Fp, SMTC1, SMFC1, SBC1T, SBC1F);
MAKE_FANCY_ENUM(FPIns, uint8_t, ADD_D = 0, SUB_D = 1, MUL_D = 2, DIV_D = 3, MOV_D = 6, CVT_D_L = 33, CVT_L_D = 37,
                C_LT_D = 60, C_LE_D = 62, C_EQ_D = 50);
MAKE_FANCY_ENUM(RegIns, uint8_t, NOP = 0, JR = 8, JALR = 9, MOVZ = 10, MOVN = 11, DSLLV = 20, DSRLV = 22, DSRAV = 23,
                DMUL = 28, DMULU = 29, DDIV = 30, DDIVU = 31, AND = 36, OR = 37, XOR = 38, SLT = 42, SLTU = 43,
                DADD = 44, DADDU = 45, DSUB = 46, DSUBU = 47, DSLL = 56, DSRL = 58, DSRA = 59);
MAKE_FANCY_ENUM(ImmIns, uint8_t, HALT = 1, J = 2, JAL = 3, BEQ = 4, BNE = 5, BEQZ = 6, BNEZ = 7, DADDI = 24,
                DADDIU = 25, SLTI = 10, SLTIU = 11, ANDI = 12, ORI = 13, XORI = 14, LUI = 15, LB = 32, LH = 33, LW = 35,
                LBU = 36, LHU = 37, LWU = 39, SB = 40, SH = 41, SW = 43, L_D = 53, S_D = 61, LD = 55, SD = 63);
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
static void decode_immediate_impl(uint32_t opcode, ImmIns ins, CPU& cpu, bool print_instructions) {
    switch (ins) {
    case ImmIns::HALT: {
        cpu.halt();
        if (print_instructions) { Printer::print("halt"); }
    } break;
    case ImmIns::J: {
        auto w = extract_j_instruction(opcode, cpu);
        cpu.move_pc(w);
        if (print_instructions) { Printer::print("j {}", w); }
    } break;
    case ImmIns::JAL: {
        constexpr auto ra_reg = 31;
        auto w = extract_j_instruction(opcode, cpu);
        cpu.reg(ra_reg, cpu.pc() + 4);
        cpu.move_pc(w);
        if (print_instructions) { Printer::print("jal {}", w); }
    } break;
    case ImmIns::BEQ: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        if (cpu.reg(rs) == cpu.reg(rt)) cpu.move_pc(w);
        if (print_instructions) { Printer::print("beq r{}, r{}, {}", rt, rs, w); }
    } break;
    case ImmIns::BNE: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        if (cpu.reg(rs) != cpu.reg(rt)) cpu.move_pc(w);
        if (print_instructions) { Printer::print("bne r{}, r{}, {}", rt, rs, w); }
    } break;
    case ImmIns::BEQZ: {
        auto [_, rt, w] = extract_i_instruction(opcode);
        w *= 4;
        if (cpu.reg(rt) == 0) cpu.move_pc(w);
        if (print_instructions) { Printer::print("beqz r{}, {}", rt, w); }
    } break;
    case ImmIns::BNEZ: {
        auto [_, rt, w] = extract_i_instruction(opcode);
        w *= 4;
        if (cpu.reg(rt) != 0) cpu.move_pc(w);
        if (print_instructions) { Printer::print("bnez r{}, {}", rt, w); }
    } break;
    case ImmIns::DADDI: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.reg(rt, cpu.reg(rs) + static_cast<uint64_t>(w));
        if (print_instructions) { Printer::print("daddi r{}, r{}, {}", rt, rs, w); }
    } break;
    case ImmIns::DADDIU: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.reg(rt, cpu.reg(rs) + static_cast<uint64_t>(w));
        if (print_instructions) { Printer::print("daddiu r{}, r{}, {}", rt, rs, w); }
    } break;
    case ImmIns::SLTI: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.reg(rt, static_cast<int64_t>(cpu.reg(rs)) < static_cast<int64_t>(w));
        if (print_instructions) { Printer::print("slti r{}, r{}, {}", rt, rs, w); }
    } break;
    case ImmIns::SLTIU: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.reg(rt, cpu.reg(rs) < static_cast<uint64_t>(w));
        if (print_instructions) { Printer::print("sltiu r{}, r{}, {}", rt, rs, w); }
    } break;
    case ImmIns::ANDI: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.reg(rt, cpu.reg(rs) & static_cast<uint64_t>(w));
        if (print_instructions) { Printer::print("andi r{}, r{}, {}", rt, rs, w); }
    } break;
    case ImmIns::ORI: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.reg(rt, cpu.reg(rs) | static_cast<uint64_t>(w));
        if (print_instructions) { Printer::print("ori r{}, r{}, {}", rt, rs, w); }
    } break;
    case ImmIns::XORI: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.reg(rt, cpu.reg(rs) ^ static_cast<uint64_t>(w));
        if (print_instructions) { Printer::print("xori r{}, r{}, {}", rt, rs, w); }
    } break;
    case ImmIns::LUI: {
        auto [_, rt, w] = extract_i_instruction(opcode);
        cpu.reg(rt, cpu.reg(rt) | (static_cast<uint64_t>(w) << 32));
        if (print_instructions) { Printer::print("lui r{}, {}", rt, w); }
    } break;
    case ImmIns::LB: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        ARLib::int8_t val = cpu.read<1>(cpu.reg(rs) + static_cast<uint64_t>(w));
        cpu.reg(rt, static_cast<uint64_t>(val));
        if (print_instructions) { Printer::print("lb r{}, {}(r{})", rt, w, rs); }
    } break;
    case ImmIns::LH: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        ARLib::int16_t val = cpu.read<2>(cpu.reg(rs) + static_cast<uint64_t>(w));
        cpu.reg(rt, static_cast<uint64_t>(val));
        if (print_instructions) { Printer::print("lh r{}, {}(r{})", rt, w, rs); }
    } break;
    case ImmIns::LW: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        ARLib::int32_t val = cpu.read<4>(cpu.reg(rs) + static_cast<uint64_t>(w));
        cpu.reg(rt, static_cast<uint64_t>(val));
        if (print_instructions) { Printer::print("lw r{}, {}(r{})", rt, w, rs); }
    } break;
    case ImmIns::LBU: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.reg(rt, cpu.read<1>(cpu.reg(rs) + static_cast<uint64_t>(w)));
        if (print_instructions) { Printer::print("lbu r{}, {}(r{})", rt, w, rs); }
    } break;
    case ImmIns::LHU: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.reg(rt, cpu.read<2>(cpu.reg(rs) + static_cast<uint64_t>(w)));
        if (print_instructions) { Printer::print("lhu r{}, {}(r{})", rt, w, rs); }
    } break;
    case ImmIns::LWU: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.reg(rt, cpu.read<4>(cpu.reg(rs) + static_cast<uint64_t>(w)));
        if (print_instructions) { Printer::print("lwu r{}, {}(r{})", rt, w, rs); }
    } break;
    case ImmIns::SB: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.write<1>(cpu.reg(rs) + static_cast<uint64_t>(w), cpu.reg(rt));
        if (print_instructions) { Printer::print("sb r{}, {}(r{})", rt, w, rs); }
    } break;
    case ImmIns::SH: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.write<2>(cpu.reg(rs) + static_cast<uint64_t>(w), cpu.reg(rt));
        if (print_instructions) { Printer::print("sh r{}, {}(r{})", rt, w, rs); }
    } break;
    case ImmIns::SW: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.write<4>(cpu.reg(rs) + static_cast<uint64_t>(w), cpu.reg(rt));
        if (print_instructions) { Printer::print("sw r{}, {}(r{})", rt, w, rs); }
    } break;
    case ImmIns::L_D: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.freg(rt, cpu.readf<8>(cpu.reg(rs) + static_cast<uint64_t>(w)));
        if (print_instructions) { Printer::print("l.d f{}, {}(r{})", rt, w, rs); }
    } break;
    case ImmIns::S_D: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.writef<8>(cpu.reg(rs) + static_cast<uint64_t>(w), cpu.freg(rt));
        if (print_instructions) { Printer::print("s.d f{}, {}(r{})", rt, w, rs); }
    } break;
    case ImmIns::LD: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        ARLib::int64_t val = cpu.read<8>(cpu.reg(rs) + static_cast<uint64_t>(w));
        cpu.reg(rt, static_cast<uint64_t>(val));
        if (print_instructions) { Printer::print("ld r{}, {}(r{})", rt, w, rs); }
    } break;
    case ImmIns::SD: {
        auto [rs, rt, w] = extract_i_instruction(opcode);
        cpu.write<8>(cpu.reg(rs) + static_cast<uint64_t>(w), cpu.reg(rt));
        if (print_instructions) { Printer::print("sd r{}, {}(r{})", rt, w, rs); }
    } break;
    }
}
static void decode_register_impl(uint32_t opcode, RegIns ins, CPU& cpu, bool print_instructions) {
    switch (ins) {
    case RegIns::NOP: {
        if (print_instructions) { Printer::print("nop"); }
        // DONE
    } break;
    case RegIns::JR: {
        auto [_, rt, __] = extract_r_instruction(opcode);
        cpu.set_pc(cpu.reg(rt) - 4);
        if (print_instructions) { Printer::print("jr r{}", rt); }
    } break;
    case RegIns::JALR: {
        constexpr auto ra_reg = 31;
        auto [_, rt, __] = extract_r_instruction(opcode);
        cpu.reg(ra_reg, cpu.pc() + 4);
        cpu.set_pc(cpu.reg(rt) - 4);
        if (print_instructions) { Printer::print("jalr r{}", rt); }
    } break;
    case RegIns::MOVZ: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        if (cpu.reg(rt) == 0) cpu.reg(rd, cpu.reg(rs));
        if (print_instructions) { Printer::print("movz r{}, r{}, r{}", rd, rs, rt); }
    } break;
    case RegIns::MOVN: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        if (cpu.reg(rt) != 0) cpu.reg(rd, cpu.reg(rs));
        if (print_instructions) { Printer::print("movn r{}, r{}, r{}", rd, rs, rt); }
    } break;
    case RegIns::DSLLV: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        auto shamt = cpu.reg(rt);
        cpu.reg(rd, cpu.reg(rs) << shamt);
        if (print_instructions) { Printer::print("dsllv r{}, r{}, r{}", rd, rs, rt); }
    } break;
    case RegIns::DSRLV: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        auto shamt = cpu.reg(rt);
        cpu.reg(rd, cpu.reg(rs) >> shamt);
        if (print_instructions) { Printer::print("dsrlv r{}, r{}, r{}", rd, rs, rt); }
    } break;
    case RegIns::DSRAV: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        auto shamt = cpu.reg(rt);
        auto sign = cpu.reg(rs) & (1ull << 63);
        cpu.reg(rd, (cpu.reg(rs) >> shamt) | sign);
        if (print_instructions) { Printer::print("dsrav r{}, r{}, r{}", rd, rs, rt); }
    } break;
    case RegIns::DMUL: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        int64_t mul = static_cast<int64_t>(cpu.reg(rs)) * static_cast<int64_t>(cpu.reg(rt));
        cpu.reg(rd, static_cast<uint64_t>(mul));
        if (print_instructions) { Printer::print("dmul r{}, r{}, r{}", rd, rs, rt); }
    } break;
    case RegIns::DMULU: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        cpu.reg(rd, cpu.reg(rs) * cpu.reg(rt));
        if (print_instructions) { Printer::print("dmulu r{}, r{}, r{}", rd, rs, rt); }
    } break;
    case RegIns::DDIV: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        if (cpu.reg(rt) == 0)
            cpu.reg(rd, 0); // divide by 0
        else {
            int64_t div = static_cast<int64_t>(cpu.reg(rs)) / static_cast<int64_t>(cpu.reg(rt));
            cpu.reg(rd, static_cast<uint64_t>(div));
        }
        if (print_instructions) { Printer::print("ddiv r{}, r{}, r{}", rd, rs, rt); }
    } break;
    case RegIns::DDIVU: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        if (cpu.reg(rt) == 0)
            cpu.reg(rd, 0); // divide by 0
        else
            cpu.reg(rd, cpu.reg(rs) / cpu.reg(rt));
        if (print_instructions) { Printer::print("ddivu r{}, r{}, r{}", rd, rs, rt); }
    } break;
    case RegIns::AND: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        cpu.reg(rd, cpu.reg(rs) & cpu.reg(rt));
        if (print_instructions) { Printer::print("and r{}, r{}, r{}", rd, rs, rt); }
    } break;
    case RegIns::OR: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        cpu.reg(rd, cpu.reg(rs) | cpu.reg(rt));
        if (print_instructions) { Printer::print("or r{}, r{}, r{}", rd, rs, rt); }
    } break;
    case RegIns::XOR: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        cpu.reg(rd, cpu.reg(rs) ^ cpu.reg(rt));
        if (print_instructions) { Printer::print("xor r{}, r{}, r{}", rd, rs, rt); }
    } break;
    case RegIns::SLT: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        cpu.reg(rd, cpu.reg(rs) < cpu.reg(rt));
        if (print_instructions) { Printer::print("slt r{}, r{}, r{}", rd, rs, rt); }
    } break;
    case RegIns::SLTU: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        cpu.reg(rd, cpu.reg(rs) < cpu.reg(rt));
        if (print_instructions) { Printer::print("sltu r{}, r{}, r{}", rd, rs, rt); }
    } break;
    case RegIns::DADD: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        cpu.reg(rd, static_cast<int64_t>(cpu.reg(rs)) + static_cast<int64_t>(cpu.reg(rt)));
        if (print_instructions) { Printer::print("dadd r{}, r{}, r{}", rd, rs, rt); }
    } break;
    case RegIns::DADDU: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        cpu.reg(rd, cpu.reg(rs) + cpu.reg(rt));
        if (print_instructions) { Printer::print("daddu r{}, r{}, r{}", rd, rs, rt); }
    } break;
    case RegIns::DSUB: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        cpu.reg(rd, static_cast<int64_t>(cpu.reg(rs)) - static_cast<int64_t>(cpu.reg(rt)));
        if (print_instructions) { Printer::print("dsub r{}, r{}, r{}", rd, rs, rt); }
    } break;
    case RegIns::DSUBU: {
        auto [rs, rt, rd] = extract_r_instruction(opcode);
        cpu.reg(rd, cpu.reg(rs) - cpu.reg(rt));
        if (print_instructions) { Printer::print("subu r{}, r{}, r{}", rd, rs, rt); }
    } break;
    case RegIns::DSLL: {
        auto [rs, _, rd] = extract_r_instruction(opcode);
        auto shamt = (opcode >> 6) & 0b11111;
        cpu.reg(rd, cpu.reg(rs) << shamt);
        if (print_instructions) { Printer::print("dsll r{}, r{}, {}", rd, rs, shamt); }
    } break;
    case RegIns::DSRL: {
        auto [rs, _, rd] = extract_r_instruction(opcode);
        auto shamt = (opcode >> 6) & 0b11111;
        cpu.reg(rd, cpu.reg(rs) >> shamt);
        if (print_instructions) { Printer::print("dsrl r{}, r{}, {}", rd, rs, shamt); }
    } break;
    case RegIns::DSRA: {
        auto [rs, _, rd] = extract_r_instruction(opcode);
        auto shamt = (opcode >> 6) & 0b11111;
        auto sign = (cpu.reg(rs) & (1ull << 63));
        cpu.reg(rd, (cpu.reg(rs) >> shamt) | sign);
        if (print_instructions) { Printer::print("dsra r{}, r{}, {}", rd, rs, shamt); }
    } break;
    }
}
static void decode_fp_impl(uint32_t opcode, FPIns ins, CPU& cpu, bool print_instructions) {
    auto [rs, rt, rd] = extract_fp_regs_from_instruction(opcode);
    switch (ins) {
    case FPIns::ADD_D: {
        cpu.freg(rd, cpu.freg(rs) + cpu.freg(rt));
        if (print_instructions) { Printer::print("add.d f{}, f{}, f{}", rs, rt, rd); }
    } break;
    case FPIns::SUB_D: {
        cpu.freg(rd, cpu.freg(rs) - cpu.freg(rt));
        if (print_instructions) { Printer::print("sub.d f{}, f{}, f{}", rs, rt, rd); }
    } break;
    case FPIns::MUL_D: {
        cpu.freg(rd, cpu.freg(rs) * cpu.freg(rt));
        if (print_instructions) { Printer::print("mul.d f{}, f{}, f{}", rs, rt, rd); }
    } break;
    case FPIns::DIV_D: {
        cpu.freg(rd, cpu.freg(rs) / cpu.freg(rt));
        if (print_instructions) { Printer::print("div.d f{}, f{}, f{}", rs, rt, rd); }
    } break;
    case FPIns::MOV_D: {
        cpu.freg(rd, cpu.freg(rs));
        if (print_instructions) { Printer::print("mov.d f{}, f{}", rd, rs); }
    } break;
    case FPIns::CVT_D_L: {
        // convert 64-bit integer to a double FP format
        double orig = cpu.freg(rs);
        uint64_t val = BitCast<uint64_t>(orig);
        cpu.freg(rd, static_cast<double>(val));
        if (print_instructions) { Printer::print("cvt.d.l f{}, f{}", rd, rs); }
    } break;
    case FPIns::CVT_L_D: {
        // convert double FP to a 64-bit integer format
        uint64_t orig = static_cast<uint64_t>(cpu.freg(rs));
        double val = BitCast<double>(orig);
        cpu.freg(rd, val);
        if (print_instructions) { Printer::print("cvt.l.d f{}, f{}", rd, rs); }
    } break;
    case FPIns::C_LT_D: {
        cpu.fpflag(cpu.freg(rs) < cpu.freg(rt));
        if (print_instructions) { Printer::print("c.lt.d f{}, f{}", rs, rt); }
    } break;
    case FPIns::C_LE_D: {
        cpu.fpflag(cpu.freg(rs) <= cpu.freg(rt));
        if (print_instructions) { Printer::print("c.le.d f{}, f{}", rs, rt); }
    } break;
    case FPIns::C_EQ_D: {
        cpu.fpflag(cpu.freg(rs) == cpu.freg(rt));
        if (print_instructions) { Printer::print("c.eq.d f{}, f{}", rs, rt); }
    } break;
    }
}

static void decode_impl(uint32_t opcode, InsType type, uint32_t num, CPU& cpu, bool print_instructions) {
    switch (type) {
    case InsType::Imm: {
        ImmIns ins = static_cast<ImmIns>(num);
        decode_immediate_impl(opcode, ins, cpu, print_instructions);
    } break;
    case InsType::Reg: {
        RegIns ins = static_cast<RegIns>(num);
        decode_register_impl(opcode, ins, cpu, print_instructions);
    } break;
    case InsType::Fp: {
        FPIns ins = static_cast<FPIns>(num);
        decode_fp_impl(opcode, ins, cpu, print_instructions);
    } break;
    case InsType::SMTC1: {
        // move data from integer register to FP register
        auto [rt, rd] = extract_m_instruction(opcode);
        cpu.freg(rd, static_cast<double>(cpu.reg(rt)));
        if (print_instructions) { Printer::print("mtc1 r{}, f{}", rt, rd); }
    } break;
    case InsType::SMFC1: {
        // move data from FP register to integer register
        auto [rt, rd] = extract_m_instruction(opcode);
        cpu.reg(rt, static_cast<uint64_t>(cpu.freg(rd)));
        if (print_instructions) { Printer::print("mfc1 r{}, f{}", rt, rd); }
    } break;
    case InsType::SBC1T: {
        int16_t w = extract_b_instruction(opcode, cpu);
        if (cpu.fpflag()) cpu.move_pc(w);
        if (print_instructions) { Printer::print("bc1t {}", w); }
    } break;
    case InsType::SBC1F: {
        int16_t w = extract_b_instruction(opcode, cpu);
        if (!cpu.fpflag()) cpu.move_pc(w);
        if (print_instructions) { Printer::print("bc1f {}", w); }
    } break;
    }
}
void Instruction::decode(CPU& cpu, bool print_instructions) {
    uint32_t instruction_id = (opcode >> 26) & 0b111111;
    uint32_t bits_check = (opcode >> 21) & 0b11111;
    if (instruction_id == 0) {
        // I_SPECIAL (SR)
        instruction_id = opcode & 0b111111;
        decode_impl(opcode, InsType::Reg, instruction_id, cpu, print_instructions);
    } else if (instruction_id == 0x11 && bits_check == 0x11) {
        // I_COP1 + I_DOUBLE (SF)
        instruction_id = opcode & 0b111111;
        // floating point instructions
        decode_impl(opcode, InsType::Fp, instruction_id, cpu, print_instructions);
    } else if (instruction_id == 0x11) {
        // special cases
        if (bits_check == 0x04) {
            // SMTC1
            decode_impl(opcode, InsType::SMTC1, instruction_id, cpu, print_instructions);
        } else if (bits_check == 0x08 && (opcode & (1 << 16))) {
            // SBC1T
            decode_impl(opcode, InsType::SBC1T, instruction_id, cpu, print_instructions);
        } else if (bits_check == 0x08) {
            // SBC1F
            decode_impl(opcode, InsType::SBC1F, instruction_id, cpu, print_instructions);
        } else {
            // SMFC1
            decode_impl(opcode, InsType::SMFC1, instruction_id, cpu, print_instructions);
        }
    } else {
        // SI instructions
        decode_impl(opcode, InsType::Imm, instruction_id, cpu, print_instructions);
    }
}