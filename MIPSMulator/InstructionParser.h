#pragma once

#include <CharConv.h>
#include <File.h>
#include <Path.h>
#include <PrintInfo.h>
#include <Types.h>
#include <Vector.h>


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
    using MixResult = Result<String, Variant<OpenFileError, ReadFileError>>;
    Vector<Instruction> instructions;
    InstructionData() = default;
    MixResult load(const Path& p);
};