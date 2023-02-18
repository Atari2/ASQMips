#pragma once

#include <Array.h>
#include <CharConv.h>
#include <EnumHelpers.h>
#include <File.h>
#include <Path.h>
#include <Printer.h>
#include <StringView.h>
#include <Vector.h>

using namespace ARLib;

MAKE_FANCY_ENUM(TokenKind, Invalid, Identifier, Label, Directive, Integer, Real, String, Char, Colon, Comma, Dot,
                OpenParens, CloseParens, Quote, Apostrophe);

struct Sep {
    const char c;
    const TokenKind t;
    constexpr bool operator==(const char cc) const noexcept { return c == cc; }
    constexpr bool operator==(const Sep& o) const noexcept { return c == o.c && t == o.t; }
};

constexpr Array separators{Sep{':', TokenKind::Colon},       Sep{',', TokenKind::Comma},
                           Sep{'.', TokenKind::Dot},         Sep{'(', TokenKind::OpenParens},
                           Sep{')', TokenKind::CloseParens}, Sep{'"', TokenKind::Quote},
                           Sep{'\'', TokenKind::Apostrophe}};

class Token {
    StringView m_token;
    TokenKind m_kind;
    StringView m_source_file;
    size_t m_line;
    size_t m_column;

    public:
    constexpr Token() noexcept = default;
    constexpr Token(const StringView& token, TokenKind kind, const StringView& source_file, size_t line,
                    size_t column) noexcept :
        m_token(token), m_kind(kind), m_source_file(source_file), m_line(line), m_column(column) {}
    constexpr Token(const Token&) noexcept = default;
    constexpr Token(Token&&) noexcept = default;
    constexpr Token& operator=(const Token& o) noexcept {
        m_token = o.token();
        m_kind = o.kind();
        m_source_file = o.source_file();
        m_line = o.line();
        m_column = o.column();
        return *this;
    }
    constexpr Token& operator=(Token&& o) noexcept {
        m_token = move(o.token());
        m_kind = o.kind();
        m_source_file = move(o.source_file());
        m_line = o.line();
        m_column = o.column();
        return *this;
    }
    constexpr const StringView& token() const noexcept { return m_token; }
    constexpr TokenKind kind() const noexcept { return m_kind; }
    constexpr const StringView& source_file() const noexcept { return m_source_file; }
    constexpr size_t line() const noexcept { return m_line; }
    constexpr size_t column() const noexcept { return m_column; }
    constexpr void set_as_label() noexcept { m_kind = TokenKind::Label; }
};

using tit = ConstIterator<Token>;

struct TokenizeError {
    TokenizeError() = default;
    constexpr static inline StringView error = "error during tokenization"_sv;
};

template <>
struct ARLib::PrintInfo<TokenizeError> {
    const TokenizeError& m_error;
    constexpr PrintInfo(const TokenizeError& error) noexcept : m_error(error) {}
    String repr() const { return "TokenizerError { "_s + m_error.error.extract_string() + " }"_s; }
};

using TokenizeResult = DiscardResult<Variant<ReadFileError, TokenizeError>>;

class Tokenizer {
    Vector<Token> m_tokens{};
    Vector<String> m_lines{};
    File m_file;
    String m_source_file;
    void print_error(const StringView& error, const Token& tok) const;
    friend class Parser;

    public:
    Tokenizer(const Path& filename) : m_file(filename), m_source_file(m_file.name().narrow()) {}
    DiscardResult<OpenFileError> open();
    TokenizeResult tokenize();
    const Vector<Token>& tokens() const { return m_tokens; }
};

template <>
struct ARLib::PrintInfo<Token> {
    const Token& m_token;
    constexpr PrintInfo(const Token& token) noexcept : m_token(token) {}
    String repr() const {
        return Printer::format("Token {{ {} \"{}\", in {} at {}:{} }}", enum_to_str_view(m_token.kind()),
                               m_token.token(), m_token.source_file(), m_token.line(), m_token.column());
    }
};