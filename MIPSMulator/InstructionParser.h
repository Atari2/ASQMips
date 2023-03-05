#pragma once

#include <Path.h>
#include <Types.h>
#include <Vector.h>

using namespace ARLib;

struct Instruction {
    uint32_t opcode;
    void decode();
};

struct InstructionData {
    Vector<Instruction> instructions;
    InstructionData(const Path& p);
};