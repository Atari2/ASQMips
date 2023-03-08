#pragma once
#include "DataParser.h"
#include "InstructionParser.h"
#include <Array.h>

class CPU {
    InstructionData m_ins_data;
    BinaryData m_ro_data;
    uint64_t m_pc{0};
    Array<uint64_t, 32> m_regs{};
    Array<double, 32> m_freg{};
    bool m_halted = false;
    uint64_t m_clock_count{0}; // inaccurate for now
    FILE* m_log_file = nullptr;

    public:
    CPU(const Path& ins_path, const Path& ro_path) : m_ins_data(ins_path), m_ro_data(ro_path) {
        FsString p{"dump.txt"};
        m_log_file = fopen(p.data(), "w");
    }
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
    template <size_t S>
    auto read(uint64_t addr) {
        if constexpr (S == 1) {
            uint8_t val = 0;
            memcpy(&val, m_ro_data.data_raw() + addr, sizeof(uint8_t));
            return val;
        } else if constexpr (S == 2) {
            uint16_t val = 0;
            memcpy(&val, m_ro_data.data_raw() + addr, sizeof(uint16_t));
            return val;
        } else if constexpr (S == 4) {
            uint32_t val = 0;
            memcpy(&val, m_ro_data.data_raw() + addr, sizeof(uint32_t));
            return val;
        } else if constexpr (S == 8) {
            uint64_t val = 0;
            memcpy(&val, m_ro_data.data_raw() + addr, sizeof(uint64_t));
            return val;
        } else {
            COMPTIME_ASSERT("Invalid size");
        }
    }
    template <size_t S>
    auto readf(uint64_t addr) {
        if constexpr (S == 4) {
            float val = 0;
            memcpy(&val, m_ro_data.data_raw() + addr, sizeof(float));
            return val;
        } else if constexpr (S == 8) {
            double val = 0;
            memcpy(&val, m_ro_data.data_raw() + addr, sizeof(double));
            return val;
        } else {
            COMPTIME_ASSERT("Invalid size");
        }
    }
    template <size_t S>
    void write(uint64_t addr, Integral auto val) {
        if constexpr (S == 1) {
            m_ro_data.data[addr] = static_cast<uint8_t>(val);
        } else if constexpr (S == 2) {
            m_ro_data.data[addr] = static_cast<uint8_t>(val & 0xFF);
            m_ro_data.data[addr + 1] = static_cast<uint8_t>((val >> 8) & 0xFF);
        } else if constexpr (S == 4) {
            m_ro_data.data[addr] = static_cast<uint8_t>(val & 0xFF);
            m_ro_data.data[addr + 1] = static_cast<uint8_t>((val >> 8) & 0xFF);
            m_ro_data.data[addr + 2] = static_cast<uint8_t>((val >> 16) & 0xFF);
            m_ro_data.data[addr + 3] = static_cast<uint8_t>((val >> 24) & 0xFF);
        } else if constexpr (S == 8) {
            m_ro_data.data[addr] = static_cast<uint8_t>(val & 0xFF);
            m_ro_data.data[addr + 1] = static_cast<uint8_t>((val >> 8) & 0xFF);
            m_ro_data.data[addr + 2] = static_cast<uint8_t>((val >> 16) & 0xFF);
            m_ro_data.data[addr + 3] = static_cast<uint8_t>((val >> 24) & 0xFF);
            m_ro_data.data[addr + 4] = static_cast<uint8_t>((val >> 32) & 0xFF);
            m_ro_data.data[addr + 5] = static_cast<uint8_t>((val >> 40) & 0xFF);
            m_ro_data.data[addr + 6] = static_cast<uint8_t>((val >> 48) & 0xFF);
            m_ro_data.data[addr + 7] = static_cast<uint8_t>((val >> 56) & 0xFF);
        } else {
            COMPTIME_ASSERT("Invalid size");
        }
    }
    template <size_t S>
    void writef(uint64_t addr, FloatingPoint auto val) {
        if constexpr (S == 4) {
            uint32_t v = BitCast(val);
            write<4>(addr, v);
        } else if constexpr (S == 8) {
            uint64_t v = BitCast(val);
            write<8>(addr, v);
        } else {
            COMPTIME_ASSERT("Invalid size");
        }
    }
    void halt() { m_halted = true; }
    auto memory() const { return m_ro_data.data; }
    void run();
    void dump_state();
    void dump_memory();
    ~CPU() {
        if (m_log_file) fclose(m_log_file);
    }
};