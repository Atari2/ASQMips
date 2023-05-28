#pragma once

#include <Array.hpp>
#include <CharConv.hpp>
#include <EnumHelpers.hpp>
#include <File.hpp>
#include <Path.hpp>
#include <Printer.hpp>
#include <StringView.hpp>
#include <Vector.hpp>

using namespace ARLib;

MAKE_FANCY_ENUM(TokenKind, uint8_t, Invalid, Identifier, Label, Directive, Integer, Real, String, Char, Colon, Comma,
                Dot, OpenParens, CloseParens, Quote, Apostrophe);

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
    FsStringView m_source_file;
    size_t m_line;
    size_t m_column;

    public:
    constexpr Token() noexcept = default;
    constexpr Token(const StringView& token, TokenKind kind, const FsStringView& source_file, size_t line,
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
    constexpr const FsStringView& source_file() const noexcept { return m_source_file; }
    constexpr size_t line() const noexcept { return m_line; }
    constexpr size_t column() const noexcept { return m_column; }
    constexpr void set_as_label() noexcept { m_kind = TokenKind::Label; }
};

using tit = ConstIterator<Token>;

class TokenizeError : public Error {
    public:
    TokenizeError(ConvertibleTo<String> auto val) : Error{move(val)} {}
    template <typename OtherError>
        requires DerivedFrom<OtherError, ErrorBase>
    TokenizeError(OtherError&& other) : Error{move(other.error_string())} {}
};

using TokenizeResult = DiscardResult<TokenizeError>;

class Tokenizer {
    Vector<Token> m_tokens{};
    Vector<String> m_lines{};
    File m_file;
    Path m_source_file;
    void print_error(const StringView& error, const Token& tok) const;
    friend class Parser;

    public:
    Tokenizer(const Path& filename) : m_file(filename), m_source_file(m_file.name().narrow()) {}
    DiscardResult<FileError> open();
    TokenizeResult tokenize();
    const Vector<Token>& tokens() const { return m_tokens; }
    void dump_tokens() const;
    const auto& source_file() const { return m_source_file; }
};

template <>
struct ARLib::PrintInfo<Token> {
    const Token& m_token;
    constexpr PrintInfo(const Token& token) noexcept : m_token(token) {}
    String repr() const {
        return Printer::format("Token {{ {} \"{}\", in {} at {}:{} }}", enum_to_str_view(m_token.kind()),
                               m_token.token(), m_token.source_file(), m_token.line() + 1, m_token.column());
    }
};