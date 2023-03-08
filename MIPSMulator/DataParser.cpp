#include "DataParser.h"
#include <File.h>

BinaryData::BinaryData(const Path& p) {
    constexpr size_t wordline_sz = sizeof(uint64_t);
    auto file = File{p};
    file.open(OpenFileMode::Read);
    auto lines_or_error = file.read_all();
    if (lines_or_error.is_error()) { return; }
    auto lines = lines_or_error.to_ok();
    data.resize(lines.size() * wordline_sz);
    auto split = lines.split_view("\n");
    for (const auto& [idx, line] : split.enumerate()) {
        auto val = StrViewToU64Hexadecimal(line);
        for (size_t i = 0; i < wordline_sz; i++) {
            data[(idx * wordline_sz) + i] = ((val >> (i * 8)) & 0xFF);
        }
    }
    data.resize(0x400);
}