#include "Parser.h"
#include "DirectiveParser.h"
#include "InstructionParser.h"

using namespace ARLib;

bool Parser::assert_next_token(tit it, tit end, TokenKind kind, bool force_errors) {
    if (it == end) {
        m_has_errored = true;
        if (force_errors) { m_tokenizer.print_error("unexpected end of token stream was reached"_sv, *it); }
        return false;
    }
    if ((*it).kind() != kind) {
        m_has_errored = true;
        auto error_message =
        "Expected token of kind "_s + enum_to_str(kind) + " but got "_s + enum_to_str((*it).kind());
        if (force_errors) { m_tokenizer.print_error(error_message.view(), *it); }
        return false;
    }
    return true;
}

bool Parser::assert_next_one_of(tit it, tit end, std::initializer_list<TokenKind> kinds) {
    for (const TokenKind& kind : kinds) {
        if (assert_next_token(it, end, kind, false)) return true;
    }
    return false;
}

struct BoolSetOnce {
    bool m_value = false;
    constexpr BoolSetOnce() = default;
    constexpr operator bool() const { return m_value; }
    constexpr BoolSetOnce& operator=(bool val) {
        if (!m_value) { m_value = val; }
        return *this;
    }
};

tit Parser::parse_section_change(tit it, tit end, Section& section) {
    // look for .data or .text or .code
    const Token& ident = *it;
    if (ident.token() == "data"_sv) {
        section = Section::Data;
    } else if (ident.token() == "text"_sv || ident.token() == "code"_sv) {
        section = Section::Text;
    } else {
        m_has_errored = true;
        m_tokenizer.print_error("Expected .data, .text or code"_sv, ident);
    }
    ++it;
    return it;
}

uint8_t Parser::m_rom_data[32768]{};

tit Parser::parse_comma_separated_list(tit it, tit end, size_t value_size) {
    while (assert_next_one_of(it, end, {TokenKind::Integer, TokenKind::Real})) {
        const auto& tok = *it;
        if (tok.kind() == TokenKind::Integer) {
            uint64_t val = StrViewToInt(tok.token()) & ((1ull << (value_size * CHAR_BIT)) - 1);
            memcpy(m_rom_data + current_address, &val, value_size);
            current_address += value_size;
        } else {
            double val = StrViewToDouble(tok.token());
            memcpy(m_rom_data + current_address, &val, value_size);
            current_address += value_size;
        }
        ++it;
        if (it == end || (*it).kind() != TokenKind::Comma) break;
        ++it;
    };
    return it;
}

static uint64_t align_address(uint64_t val, uint64_t off, uint64_t align = sizeof(uint64_t)) {
    uint64_t newval = val + off;
    if (newval < align) return align;
    if (uint64_t disp = newval % align; disp != 0) { newval += (align - disp); }
    return newval;
}

tit Parser::parse_directive(tit it, tit end, Section& section) {
    if (!assert_next_token(++it, end, TokenKind::Directive)) return it;
    const auto& directive = *it;
    auto dirit =
    directive_map.find(directive.token(), [](const auto& dir, const StringView name) { return dir.name == name; });
    HARD_ASSERT(dirit != directive_map.end(), "directive not found in directive map");
    // we assume it is found because we checked for it in the tokenizer

    const auto& dir = (*dirit).val;
    switch (dir.val) {
    case DirectiveType::data:
    case DirectiveType::text:
    case DirectiveType::code:
        it = parse_section_change(it, end, section);
        break;
    case DirectiveType::org:
        if (!assert_next_token(++it, end, TokenKind::Integer)) return it;
        current_address = StrViewToInt((*it).token());
        ++it;
        break;
    case DirectiveType::align:
        if (!assert_next_token(++it, end, TokenKind::Integer)) return it;
        current_address = align_address(current_address, 0, StrViewToInt((*it).token()));
        ++it;
        break;
    case DirectiveType::space:
        if (!assert_next_token(++it, end, TokenKind::Integer)) return it;
        current_address = align_address(current_address, StrViewToInt((*it).token()));
        ++it;
        break;
    case DirectiveType::ascii:
        if (!assert_next_token(++it, end, TokenKind::String)) return it;
        memcpy(m_rom_data + current_address, (*it).token().data(), (*it).token().length());
        current_address = align_address(current_address, (*it).token().length());
        ++it;
        break;
    case DirectiveType::asciiz:
        if (!assert_next_token(++it, end, TokenKind::String)) return it;
        memcpy(m_rom_data + current_address, (*it).token().data(), (*it).token().length() + 1);
        current_address = align_address(current_address, (*it).token().length() + 1);
        ++it;
        break;
    case DirectiveType::byte:
        it = parse_comma_separated_list(++it, end, sizeof(uint8_t));
        current_address = align_address(current_address, 0);
        break;
    case DirectiveType::word:
        it = parse_comma_separated_list(++it, end, sizeof(uint64_t));
        current_address = align_address(current_address, 0);
        break;
    case DirectiveType::word32:
        it = parse_comma_separated_list(++it, end, sizeof(uint32_t));
        current_address = align_address(current_address, 0);
        break;
    case DirectiveType::word16:
        it = parse_comma_separated_list(++it, end, sizeof(uint16_t));
        current_address = align_address(current_address, 0);
        break;
    case DirectiveType::double_:
        it = parse_comma_separated_list(++it, end, sizeof(double));
        current_address = align_address(current_address, 0);
        break;
    }
    return it;
}

tit Parser::parse_instruction(tit begin, tit end) {
    auto it = begin;
    ++it;
    return it;
}

ParseResult Parser::parse() {
    BoolSetOnce has_errored{};
    Section section = Section::None;
    const auto& tokens = m_tokenizer.tokens();
    auto begin = tokens.begin();
    auto end = tokens.end();
    auto it = begin;
    while (it != end) {
        const Token& cur = *it;
        switch (section) {
        case Section::None:
            if (!assert_next_token(it, end, TokenKind::Dot)) break;
            if (!assert_next_token(++it, end, TokenKind::Directive)) break;
            it = parse_section_change(it, end, section);
            break;
        case Section::Data: {
            // look for labels or directives
            switch (cur.kind()) {
            case TokenKind::Dot:
                it = parse_directive(it, end, section);
                break;
            case TokenKind::Label:
                // add label
                const auto& ident = *it;
                if (!assert_next_token(++it, end, TokenKind::Colon)) break;
                m_labels.add(ident.token(), {ident.token(), current_address});
                ++it; // skip colon
                break;
            }
        } break;
        case Section::Text:
            // look for labels or instructions
            switch (cur.kind()) {
            case TokenKind::Identifier: {
                // parse instructions
                it = parse_instruction(it, end);
            }
            case TokenKind::Label: {
                // add label
                const auto& ident = *it;
                if (!assert_next_token(++it, end, TokenKind::Colon)) break;
                m_labels.add(ident.token(), {ident.token(), current_address});
                ++it; // skip colon
            } break;
            case TokenKind::Dot: {
                if (!assert_next_token(++it, end, TokenKind::Directive)) break;
                const auto& directive = *it;
                auto dirit = directive_map.find(
                directive.token(), [](const auto& dir, const StringView name) { return dir.name == name; });
                HARD_ASSERT(dirit != directive_map.end(), "directive not found in directive map");
                // we assume it is found because we checked for it in the tokenizer
                const auto& dir = (*dirit).val;
                switch (dir.val) {
                case DirectiveType::data:
                case DirectiveType::text:
                case DirectiveType::code:
                    it = parse_section_change(it, end, section);
                    break;
                case DirectiveType::org:
                    if (!assert_next_token(++it, end, TokenKind::Integer)) break;
                    current_address = StrViewToInt((*it).token());
                    ++it;
                    break;
                }
            } break;
            default:
                Printer::print("Unhandled token: {}", cur);
                ++it;
                break;
            }
        }
    }
    for (const auto& [name, label] : m_labels) {
        Printer::print("{}", label);
    }
    return ParseResult::from_ok();
}