#pragma once

#include <File.h>
#include <Path.h>
#include <Types.h>
#include <Vector.h>


using namespace ARLib;

struct BinaryData {
    using MixResult = Result<String, Variant<OpenFileError, ReadFileError>>;
    Vector<uint8_t> data;
    BinaryData() = default;
    MixResult load(const Path& p);
    const uint8_t* data_raw() { return data.data(); }
};