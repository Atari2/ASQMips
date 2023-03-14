#include "DataParser.h"
#include <File.h>

BinaryData::BinaryData(const Path& p) {
    constexpr size_t wordline_sz = sizeof(uint64_t);
    File file{p};
    file.open(OpenFileMode::Read);
    auto lines_or_error = file.read_all();
    if (lines_or_error.is_error()) {
        // TODO: deal with error
        return;
    }
    auto filedata = lines_or_error.to_ok();
    data.reserve(filedata.size() * wordline_sz);
    for (const auto& [idx, val] :
         filedata.view().split("\n").iter().filter(&StringView::size).map(StrViewToU64Hexadecimal).enumerate()) {
        for (size_t i = 0; i < wordline_sz; i++) {
            data.append((val >> (i * wordline_sz)) & 0xFF);
        }
    }
    data.resize(0x400);
}