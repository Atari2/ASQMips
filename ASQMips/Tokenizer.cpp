#include "Tokenizer.h"
#include "DirectiveParser.h"
#include <CxprHashMap.h>

DiscardResult<OpenFileError> Tokenizer::open() {
    TRY(m_file.open(OpenFileMode::Read));
    return DiscardResult<OpenFileError>::from_ok();
}

static StringView from_iter(ConstIterator<char> it, ConstIterator<char> end) {
    return StringView{&*it, &*end};
}
using itt = ConstIterator<char>;

static auto is_separator(char c) {
    return find_if(separators, [c](const Sep& s) { return c == s.c; });
}

static bool is_valid_identifier(const char c) {
    if (!(isalnum(c) || c == '_' || c == '$')) return false;
    return true;
}
static bool is_valid_identifier(StringView word) {
    for (char c : word) {
        if (!is_valid_identifier(c)) { return false; }
    }
    return true;
}
static auto go_until_valid_ident(itt it, itt end) {
    if (it == end) return it;
    while (!(isspace(*it) || is_separator(*it) != npos_) && is_valid_identifier(*it)) {
        ++it;
        if (it == end) return it;
    }
    return it;
}

static auto go_until_valid_number(itt it, itt end, uint8_t& dot_n) {

    auto is_valid_sep_for_number = [](itt it) {
        auto idx = is_separator(*it);
        if (idx != npos_ && separators[idx].t != TokenKind::Dot) return true;
        return false;
    };

    if (it == end) return it;
    while (!(isspace(*it) || is_valid_sep_for_number(it)) && (isdigit(*it) || *it == '.' || *it == '-')) {
        if (*it == '.') {
            if (dot_n) {
                dot_n++;
                return it;
            };
            dot_n++;
        }
        ++it;
        if (it == end) return it;
    }
    return it;
}

TokenizeResult Tokenizer::tokenize() {
    bool has_errored = false;
    auto lines_or_err = m_file.read_all();
    if (lines_or_err.is_error()) { return TokenizeResult::from_error(lines_or_err.to_error()); }
    m_lines = lines_or_err.to_ok()
              .split("\n")
              .view()
              .inplace_transform([](String& line) { return line.trim(); })
              .collect<Vector<String>>();
    for (const auto& [i, line] : m_lines.enumerate()) {
        if (line.is_empty()) continue;
        auto idx = line.index_of(';');
        StringView real_line{};
        if (idx != StringView::npos) {
            real_line = line.substringview(0, idx);
        } else {
            real_line = line.view();
        }
        auto begin = real_line.begin();
        auto end = real_line.end();
        auto it = begin;
        while (it != end) {
            while (isspace(*it))
                ++it;
            if (it == end) break;
            if (auto idx = is_separator(*it); idx != npos_) {
                auto sep = separators[idx];
                if (sep.t == TokenKind::Quote) {
                    ++it;
                    auto next_quote = advance_iterator(it, end, find(it, end, '"'));
                    auto tok = Token{from_iter(it, next_quote), TokenKind::String, m_source_file.view(), i, it - begin};
                    if (next_quote == end) {
                        print_error("unterminated string"_sv, tok);
                        has_errored = true;
                    }
                    m_tokens.append(move(tok));
                    it = next_quote;
                } else if (sep.t == TokenKind::Apostrophe) {
                    ++it;
                    auto next_apost = advance_iterator(it, end, find(it, end, '\''));
                    auto tok = Token{from_iter(it, next_apost), TokenKind::Char, m_source_file.view(), i, it - begin};
                    if (next_apost == end || (next_apost - it) != 1) {
                        print_error("unterminated character literal"_sv, tok);
                        has_errored = true;
                    }
                    m_tokens.append(move(tok));
                    it = next_apost;
                } else {
                    auto tok = Token{from_iter(it, it + 1), sep.t, m_source_file.view(), i, it - begin};
                    if (tok.kind() == TokenKind::Colon) {
                        if (m_tokens.empty()) {
                            print_error("unexpected colon without previous tokens"_sv, tok);
                            has_errored = true;
                        }
                        auto& prev = m_tokens.last();
                        if (prev.kind() == TokenKind::Identifier) {
                            prev.set_as_label();
                        } else {
                            print_error("unexpected colon after token"_sv, tok);
                            has_errored = true;
                        }
                    }
                    m_tokens.append(tok);
                }
                if (it != end) ++it;
            } else if (isdigit(*it) || *it == '-') {
                uint8_t dot_n = 0;
                auto end_of_digit = go_until_valid_number(it, end, dot_n);
                auto tok = Token{from_iter(it, end_of_digit), dot_n == 0 ? TokenKind::Integer : TokenKind::Real,
                                 m_source_file.view(), i, it - begin};
                if (*it == '-' && end_of_digit == it + 1) {
                    print_error("lone - found"_sv, tok);
                    has_errored = true;
                }
                m_tokens.append(move(tok));
                if (dot_n > 1) {
                    print_error("unexpected second decimal divider while parsing floating point number",
                                m_tokens.last());
                    has_errored = true;
                }
                it = end_of_digit;
            } else {
                auto end_of_token = go_until_valid_ident(it, end);
                const auto word = from_iter(it, end_of_token);

                // check if token is a directive
                if (auto dir_it = is_directive(word); dir_it != directive_map.end()) {
                    if (m_tokens.last().kind() == TokenKind::Dot) {
                        m_tokens.emplace(word, TokenKind::Directive, m_source_file.view(), i, it - begin);
                    } else {
                        m_tokens.emplace(word, TokenKind::Identifier, m_source_file.view(), i, it - begin);
                    }
                } else {
                    bool valid = is_valid_identifier(word);
                    m_tokens.emplace(word, valid ? TokenKind::Identifier : TokenKind::Invalid, m_source_file.view(), i,
                                     it - begin);
                    const auto& tok = m_tokens.last();
                    if (!valid) {
                        print_error("invalid identifier token", tok);
                        has_errored = true;
                    }
                }
                it = end_of_token;
            }
        }
    }
    if (has_errored) { return TokenizeResult::from_error(TokenizeError{}); }
    return TokenizeResult::from_ok();
}

void Tokenizer::print_error(const StringView& error, const Token& tok) const {
    Printer::print("error: {} at {} (full line: {})", error, tok, m_lines[tok.line()]);
}