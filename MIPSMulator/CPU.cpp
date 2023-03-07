#include "CPU.h"
#include "cstdio_compat.h"
#include <Console.h>

void CPU::run() {
    while (!m_halted) {
        auto& ins = m_ins_data.instructions[m_pc / sizeof(uint32_t)];
        ins.decode(*this);
        dump_state();
        m_pc += sizeof(uint32_t); // 1 instruction is 4 bytes, TODO: consider jumps
        m_clock_count++;
    }
}
void CPU::dump_state() {
    fprintf(m_log_file, "At clock count = %lld, pc = %lld\n", m_clock_count, m_pc);
    for (const auto& [i, t] : enumerate(zip(m_regs, m_freg))) {
        auto [reg, freg] = t;
        fprintf(m_log_file, "\tr%-2d = %016llX    f%-2d = %7.8Lf\n", i, reg, i, freg);
    }
}