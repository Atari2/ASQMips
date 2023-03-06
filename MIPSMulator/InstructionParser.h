#pragma once

#include <CharConv.h>
#include <Path.h>
#include <PrintInfo.h>
#include <Types.h>
#include <Vector.h>

using namespace ARLib;

struct Instruction {
    uint32_t opcode;
    void decode();
};

template <>
struct ARLib::PrintInfo<Instruction> {
    const Instruction& m_ins;
    PrintInfo(const Instruction& ins) : m_ins(ins) {}
    String repr() const { return IntToStr(m_ins.opcode); }
};

struct InstructionData {
    Vector<Instruction> instructions;
    InstructionData(const Path& p);
};