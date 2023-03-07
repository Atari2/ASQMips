#pragma once
#include "DataParser.h"
#include "InstructionParser.h"
#include <Array.h>

class CPU {
    InstructionData m_ins_data;
    BinaryData m_ro_data;
    uint64_t m_pc;
    Array<uint64_t, 32> m_regs{};
    Array<double, 32> m_freg{};
    bool m_halted = false;

    public:
    CPU(const Path& ins_path, const Path& ro_path) : m_ins_data(ins_path), m_ro_data(ro_path) {}
    uint64_t reg(Integral auto reg) const { return m_regs[static_cast<uint32_t>(reg)]; }
    void reg(Integral auto reg, Integral auto val) { m_regs[static_cast<uint32_t>(reg)] = static_cast<uint64_t>(val); }
    double freg(Integral auto reg) const { return m_freg[static_cast<uint32_t>(reg)]; }
    void freg(Integral auto reg, FloatingPoint auto val) {
        m_freg[static_cast<uint32_t>(reg)] = static_cast<double>(val);
    }
    uint64_t pc() const { return m_pc; }
    void move_pc(int64_t offset) {
        int64_t pc = static_cast<int64_t>(m_pc);
        pc += offset;
        m_pc = static_cast<uint64_t>(pc);
    }
    void halt() { m_halted = true; }
    auto memory() const { return m_ro_data.data; }
    void run();
    void dump_state();
};