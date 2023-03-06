#include "Parser.h"
#include "DirectiveParser.h"
#include "InstructionParser.h"

#define CHAR_BIT 8
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

tit Parser::parse_section_change(tit it, [[maybe_unused]] tit end, Section& section) {
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

uint8_t Parser::m_ro_data[32768]{};
uint8_t Parser::m_code_data[32768]{};

tit Parser::parse_comma_separated_list(tit it, tit end, size_t value_size) {
    while (assert_next_one_of(it, end, {TokenKind::Integer, TokenKind::Real})) {
        const auto& tok = *it;
        if (tok.kind() == TokenKind::Integer) {
            uint64_t val = StrViewToU64(tok.token()) & ((1ull << (value_size * CHAR_BIT)) - 1);
            memcpy(m_ro_data + current_address, &val, value_size);
            current_address += value_size;
        } else {
            double val = StrViewToDouble(tok.token());
            memcpy(m_ro_data + current_address, &val, value_size);
            current_address += value_size;
        }
        ++it;
        if (it == end || (*it).kind() != TokenKind::Comma) break;
        ++it;
    };
    return it;
}

template <Integral T>
static T align_address(T val, uint64_t off, uint64_t align = sizeof(uint64_t)) {
    T align_v = static_cast<T>(align);
    T off_v = static_cast<T>(off);
    T newval = val + off_v;
    if (newval < align_v) return align_v;
    if (T disp = newval % align_v; disp != 0) { newval += (align_v - disp); }
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
        if (section == Section::Data) {
            current_address = StrViewToU64((*it).token());
        } else {
            current_pc = StrViewToUInt((*it).token());
        }
        ++it;
        break;
    case DirectiveType::align:
        if (!assert_next_token(++it, end, TokenKind::Integer)) return it;
        if (section == Section::Data) {
            current_address = align_address(current_address, 0, StrViewToU64((*it).token()));
        } else {
            current_pc = align_address(current_pc, 0, StrViewToU64((*it).token()));
        }
        ++it;
        break;
    case DirectiveType::space:
        if (!assert_next_token(++it, end, TokenKind::Integer)) return it;
        current_address = align_address(current_address, StrViewToU64((*it).token()));
        ++it;
        break;
    case DirectiveType::ascii:
        if (!assert_next_token(++it, end, TokenKind::String)) return it;
        memcpy(m_ro_data + current_address, (*it).token().data(), (*it).token().length());
        current_address = align_address(current_address, (*it).token().length());
        ++it;
        break;
    case DirectiveType::asciiz:
        if (!assert_next_token(++it, end, TokenKind::String)) return it;
        memcpy(m_ro_data + current_address, (*it).token().data(), (*it).token().length());
        m_ro_data[current_address + (*it).token().length()] = '\0';
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

template <>
struct ARLib::cxpr::Hasher<ARLib::String> {
    constexpr uint32_t operator()(const String& str) const { return crc32::calculate(str.view()); }
};

tit Parser::parse_instruction(tit begin, tit end) {
    auto it = begin;
    const auto& tok = *it;
    ++it;
    auto ins_name = tok.token().extract_string();
    if ((*it).kind() == TokenKind::Dot) {
        ++it;
        if (!assert_next_token(it, end, TokenKind::Identifier)) return it;
        ins_name += "."_s + (*it).token().extract_string();
        ++it;
    }
    if ((*it).kind() == TokenKind::Dot) {
        ++it;
        if (!assert_next_token(it, end, TokenKind::Identifier)) return it;
        ins_name += "."_s + (*it).token().extract_string();
        ++it;
    }
    auto instit =
    instruction_map.find(ins_name, [](const auto& inst, const String& name) { return inst.name == name.view(); });
    if (instit == instruction_map.end()) {
        m_tokenizer.print_error("invalid instruction", tok);
        return it;
    }
    const auto& inst = (*instit).val;
    InstructionData data{inst};
    data.pc_address = current_pc;
    auto add_immediate = [&](const Token& arg, Immediate& imm) {
        switch (arg.kind()) {
        case TokenKind::Integer:
            imm = StrViewToInt(arg.token());
            break;
        case TokenKind::Real:
            imm = StrViewToDouble(arg.token());
            break;
        case TokenKind::Identifier:
            imm = arg.token();
            break;
        default:
            ASSERT_NOT_REACHED("invalid immediate");
            break;
        }
        ++it;
    };
    auto is_register_floating_point = [](RegisterEnum reg) {
        return (reg >= RegisterEnum::f0 && reg <= RegisterEnum::f31);
    };
    auto is_register_integer = [](RegisterEnum reg) {
        return (reg >= RegisterEnum::r0 && reg <= RegisterEnum::r31);
    };
    auto add_register = [&](const Token& arg, RegisterEnum& reg) -> RegisterEnum {
        if (!assert_next_token(it, end, TokenKind::Identifier)) return RegisterEnum::r0;
        auto regit =
        register_map.find(arg.token(), [](const auto& reg, const StringView name) { return reg.name == name; });
        if (regit == register_map.end()) {
            m_tokenizer.print_error("register not found in register map", arg);
            return RegisterEnum::r0;
        }
        const auto& regv = (*regit).val;
        reg = regv.val;
        ++it;
        return regv.val;
    };

    for (size_t i = 0; i < inst.arg_count; ++i) {
        const auto& arg = *it;
        switch (inst.arg_types[i]) {
        case ArgumentType::Imm:
            data.args[i].m_type = ArgumentType::Imm;
            add_immediate(arg, data.args[i].m_imm());
            break;
        case ArgumentType::Freg: {
            data.args[i].m_type = ArgumentType::Freg;
            auto reg = add_register(arg, data.args[i].m_reg());
            if (!is_register_floating_point(reg)) {
                m_tokenizer.print_error("register is not a floating point register", arg);
            }
        } break;
        case ArgumentType::Reg: {
            data.args[i].m_type = ArgumentType::Reg;
            auto reg = add_register(arg, data.args[i].m_reg());
            if (!is_register_integer(reg)) { m_tokenizer.print_error("register is not an integer register", arg); }
        } break;
        case ArgumentType::ImmWReg:
            add_immediate(arg, data.args[i].m_imm_reg().first());
            if (!assert_next_token(it, end, TokenKind::OpenParens)) return it;
            add_register(*(++it), data.args[i].m_imm_reg().second());
            if (!assert_next_token(it, end, TokenKind::CloseParens)) return it;
            ++it; // skip parens
            break;
        }
        if (i != inst.arg_count - 1) {
            if (!assert_next_token(it, end, TokenKind::Comma)) return it;
            ++it; // skip comma
        }
    }
    current_pc += sizeof(uint32_t);
    m_instructions.append(move(data));
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
            case TokenKind::Label: {
                // add label
                const auto& ident = *it;
                if (!assert_next_token(++it, end, TokenKind::Colon)) break;
                m_labels.add(ident.token(), {ident.token(), current_address});
                ++it; // skip colon
            } break;
            default:
                ASSERT_NOT_REACHED("invalid token in data section");
                break;
            }
        } break;
        case Section::Text:
            // look for labels or instructions
            switch (cur.kind()) {
            case TokenKind::Identifier: {
                // parse instructions
                it = parse_instruction(it, end);
            } break;
            case TokenKind::Label: {
                // add label
                const auto& ident = *it;
                if (!assert_next_token(++it, end, TokenKind::Colon)) break;
                m_labels.add(ident.token(), {ident.token(), current_pc});
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
                    current_address = StrViewToU64((*it).token());
                    ++it;
                    break;
                default:
                    ASSERT_NOT_REACHED("invalid directive in text section");
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
    for (auto& insn : m_instructions) {
        auto& info = *insn.info;
        for (size_t i = 0; i < info.arg_count; ++i) {
            auto& arg = insn.args[i];
            switch (arg.m_type) {
            case ArgumentType::Imm: {
                if (arg.m_imm().contains_type<StringView>()) {
                    const auto& label = arg.m_imm().get<StringView>();
                    auto label_info = m_labels.find(label);
                    if (label_info == m_labels.end()) {
                        Printer::print("label {} not found", label);
                        has_errored = true;
                    } else {
                        const auto& label_addr = (*label_info).value().address;
                        arg.m_imm() = static_cast<int32_t>(label_addr);
                    }
                }
            } break;
            case ArgumentType::ImmWReg: {
                if (arg.m_imm_reg().first().contains_type<StringView>()) {
                    const auto& label = arg.m_imm_reg().first().get<StringView>();
                    auto label_info = m_labels.find(label);
                    if (label_info == m_labels.end()) {
                        Printer::print("label {} not found", label);
                        has_errored = true;
                    } else {
                        const auto& label_addr = (*label_info).value().address;
                        arg.m_imm_reg().first() = static_cast<int32_t>(label_addr);
                    }
                }
            } break;
                break;
            default:
                break;
            }
        }
    }
    return ParseResult::from_ok();
}

#ifdef _MSC_VER
#define FSCHAR(x) L##x
#else
#define FSCHAR(x) x
#endif

static Path replace_extension(const Path& path, FsStringView ext) {
    FsString orig = path.extension().string();
    return path.string().replace(orig.view(), ext);
}

bool Parser::dump_binary_data() const {
    Path ro_data_bin = replace_extension(m_tokenizer.source_file(), FSCHAR(".bin"));
    Path ro_data_dat = replace_extension(m_tokenizer.source_file(), FSCHAR(".dat"));
    FILE* fp = fopen(ro_data_bin.string().data(), "wb");
    if (!fp) { return false; }
    fwrite(m_ro_data, 1, current_address, fp);
    fclose(fp);
    fp = fopen(ro_data_dat.string().data(), "w");
    const uint64_t* data = reinterpret_cast<const uint64_t*>(m_ro_data);
    for (size_t i = 0; i < current_address / sizeof(uint64_t); ++i) {
        fprintf(fp, "%016llx\n", data[i]);
    }
    fclose(fp);
    return true;
}
void Parser::dump_instructions() const {
    for (const auto& instruction : instructions()) {
        char buf[10]{};
        int ret = snprintf(buf, sizeof(buf), "0x%04X: ", instruction.address());
        buf[ret] = '\0';
        Printer::print("{}{}", StringView{buf}, instruction);
    }
}
void Parser::dump_labels() const {
    for (const auto& [name, value] : m_labels) {
        Printer::print("{}: {}", name, value);
    }
}
void Parser::encode_instructions() const {
    Path instruction_file = replace_extension(m_tokenizer.source_file(), FSCHAR(".cod"));
    FILE* fp = fopen(instruction_file.string().data(), "w");
    for (const auto& insn : m_instructions) {
        fprintf(fp, "%08x\n", insn.encode());
    }
    fclose(fp);
}