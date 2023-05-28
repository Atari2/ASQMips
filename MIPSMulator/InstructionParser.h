#pragma once

#include <CharConv.hpp>
#include <File.hpp>
#include <Path.hpp>
#include <PrintInfo.hpp>
#include <Types.hpp>
#include <Vector.hpp>

using namespace ARLib;

class CPU;

struct Instruction {
    uint32_t opcode;
    void decode(CPU& cpu, bool print_instructions);
};

template <>
struct ARLib::PrintInfo<Instruction> {
    const Instruction& m_ins;
    PrintInfo(const Instruction& ins) : m_ins(ins) {}
    String repr() const { return IntToStr(m_ins.opcode); }
};

struct InstructionData {
    Vector<Instruction> instructions;
    InstructionData() = default;
    DiscardResult<FileError> load(const Path& p);
};