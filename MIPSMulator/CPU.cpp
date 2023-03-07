#include "CPU.h"
#include "cstdio_compat.h"
#include <Console.h>

void CPU::run() {
    while (!m_halted) {
        auto& ins = m_ins_data.instructions[m_pc / sizeof(uint32_t)];
        ins.decode(*this);
        dump_state();
        m_pc += sizeof(uint32_t); // 1 instruction is 4 bytes, TODO: consider jumps
    }
}
void CPU::dump_state() {
    printf("\tpc = %lld\n", m_pc);
    for (const auto& [i, reg] : m_regs.enumerate()) {
        if (reg != 0) printf("\tr%d = %lld\n", i, reg);
    }
    for (const auto& [i, freg] : m_freg.enumerate()) {
        if (freg != 0.0) printf("\tf%d = %f\n", i, freg);
    }
}