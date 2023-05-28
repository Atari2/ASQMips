#pragma once
#include "Tokenizer.h"
#include <CxprHashMap.hpp>
#include <EnumHelpers.hpp>

using namespace ARLib;

MAKE_FANCY_ENUM(DirectiveType, uint8_t, data, text, code, org, space, asciiz, ascii, align, word, byte, word32, word16,
                double_)

struct Directive {
    StringView name;
    DirectiveType val;
    constexpr bool operator==(const Directive& other) const { return name == other.name && val == other.val; }
};

template <>
struct cxpr::Hasher<Directive> {
    constexpr uint32_t operator()(const Directive& val) const noexcept { return crc32::calculate(val.name); }
};

constexpr auto construct_directive_map() {
    cxpr::HashTable<Directive, enum_size<DirectiveType>()> table{};
    for (auto e : for_each_enum<DirectiveType>()) {
        const auto view = enum_to_str_view(e);
        if (view[view.size() - 1] == '_') {
            table.insert({view.substringview(0, view.size() - 1), e});
        } else {
            table.insert({view, e});
        }
    }
    return table;
}

constexpr auto directive_map = construct_directive_map();

constexpr auto is_directive(const StringView word) {
    return directive_map.find(word, [](const Directive& v, const StringView view) { return v.name == view; });
}

class Parser;
tit parse_directive(tit begin, tit end, Parser& parser);
