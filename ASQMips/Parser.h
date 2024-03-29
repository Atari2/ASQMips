#pragma once
#include "InstructionParser.h"
#include "Tokenizer.h"
#include <FlatMap.hpp>
#include <SSOVector.hpp>

using namespace ARLib;

struct InstructionInfo;

enum class Section { None, Data, Text };

class Parser {
    static uint8_t m_ro_data[32768];
    static uint8_t m_code_data[32768];
    FlatMap<StringView, Label> m_labels;
    Vector<InstructionData> m_instructions;
    Tokenizer& m_tokenizer;
    bool m_has_errored = false;
    uint64_t current_address = 0;
    uint32_t current_pc = 0;
    bool assert_next_token(tit it, tit end, TokenKind kind, bool force_errors = true);
    tit parse_section_change(tit it, tit end, Section& section);
    tit parse_directive(tit it, tit end, Section& section);
    tit parse_instruction(tit begin, tit end);
    tit parse_comma_separated_list(tit it, tit end, size_t value_size);
    bool assert_next_one_of(tit it, tit end, std::initializer_list<TokenKind> kinds);

    public:
    Parser(Tokenizer& tokenizer) : m_tokenizer(tokenizer) {
        memset(m_ro_data, 0, sizeof(m_ro_data));
        memset(m_code_data, 0, sizeof(m_code_data));
    }
    DiscardResult<> parse();
    const auto& instructions() const { return m_instructions; }
    bool dump_binary_data() const;
    void dump_instructions() const;
    void dump_labels() const;
    void encode_instructions() const;
};