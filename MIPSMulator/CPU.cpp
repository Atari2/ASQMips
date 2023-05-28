#include "CPU.h"
#include <Console.hpp>
#include <cstdio_compat.hpp>

void CPU::run(bool print_instructions) {
    while (!m_halted) {
        auto& ins = m_ins_data.instructions[m_pc / sizeof(uint32_t)];
        ins.decode(*this, print_instructions);
        dump_state();
        m_pc += sizeof(uint32_t);
        m_clock_count++;
    }
    dump_memory();
}
void CPU::dump_state() {
    fprintf(m_log_file, "At clock count = %lld, pc = %lld\n", m_clock_count, m_pc);
    for (const auto& [i, t] : enumerate(zip(m_regs, m_freg))) {
        auto [reg, freg] = t;
        fprintf(m_log_file, "\tr%-2d = %016llX    f%-2d = %016.8lf\n", i, reg, i, freg);
    }
}

void CPU::dump_memory() {
    File f{Path{"memdump.dat"}};
    f.open(OpenFileMode::Write);
    size_t words = m_ro_data.data.size() / sizeof(uint64_t);
    static char buf[1024];
    for (size_t i = 0; i < words; i++) {
        buf[0] = '\0';
        uint64_t val = 0;
        memcpy(&val, m_ro_data.data_raw() + (i * sizeof(uint64_t)), sizeof(uint64_t));
        size_t sz = static_cast<size_t>(snprintf(buf, sizeof(buf), "%04X %016llX\n", i * sizeof(uint64_t), val));
        f.write(String{buf, sz});
    }
}