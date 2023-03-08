#pragma once

#include <Path.h>
#include <Types.h>
#include <Vector.h>

using namespace ARLib;

struct BinaryData {
    Vector<uint8_t> data;
    BinaryData(const Path& p);
    const uint8_t* data_raw() { return data.data(); }
};