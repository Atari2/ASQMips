#pragma once

#include <Path.h>
#include <Types.h>
#include <Vector.h>

using namespace ARLib;

struct BinaryData {
    Vector<uint64_t> data;
    BinaryData(const Path& p);
};