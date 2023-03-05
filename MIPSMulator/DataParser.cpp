#include "DataParser.h"
#include <File.h>

BinaryData::BinaryData(const Path& p) {
    auto file = File{p};
    file.open(OpenFileMode::Read);
    auto lines_or_error = file.read_all();
    if (lines_or_error.is_error()) { return; }
    auto lines = lines_or_error.to_ok();
    for (auto& line : lines.split_view("\n")) {
        data.append(StrViewToU64Hexadecimal(line));
    }
}