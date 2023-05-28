#pragma once

#include <File.hpp>
#include <Path.hpp>
#include <Types.hpp>
#include <Vector.hpp>

using namespace ARLib;

struct BinaryData {
    Vector<uint8_t> data;
    BinaryData() = default;
    DiscardResult<FileError> load(const Path& p);
    const uint8_t* data_raw() { return data.data(); }
};