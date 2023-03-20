#include "DataParser.h"

BinaryData::MixResult BinaryData::load(const Path& p) {
    constexpr size_t wordline_sz = sizeof(uint64_t);
    auto lines_or_error = File::read_all(p);
    if (lines_or_error.is_error()) { return MixResult::from_error(lines_or_error.to_error()); }
    auto filedata = lines_or_error.to_ok();
    data.reserve(filedata.size() * wordline_sz);
    for (const auto& [idx, val] :
         filedata.view().split("\n").iter().filter(&StringView::size).map(StrViewToU64Hexadecimal).enumerate()) {
        for (size_t i = 0; i < wordline_sz; i++) {
            data.append((val >> (i * wordline_sz)) & 0xFF);
        }
    }
    data.resize(0x400);
    return MixResult::from_ok();
}