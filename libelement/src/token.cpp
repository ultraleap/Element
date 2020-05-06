#include "token_internal.hpp"

#include <cctype>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include "utf8.h"

#include <fmt/format.h>

#include "interpreter_internal.hpp"

//#define INCREMENT_TOKEN_LEN(s) { ++((s)->pos); ++((s)->col); ++((s)->cur_token.tok_len); }

// #define UTF8_UNCHECKED
#if defined(UTF8_UNCHECKED)
#define UTF8_PEEK_NEXT(it, end)  utf8::unchecked::peek_next(it)
#define UTF8_NEXT(it, end)       utf8::unchecked::next(it)
#define UTF8_ADVANCE(it, n, end) utf8::unchecked::advance(it, n)
#else
#define UTF8_PEEK_NEXT(it, end)  utf8::peek_next(it, end)
#define UTF8_NEXT(it, end)       utf8::next(it, end)
#define UTF8_ADVANCE(it, n, end) utf8::advance(it, n, end)
#endif


element_result element_tokeniser_get_filename(const element_tokeniser_ctx* state, const char** filename)
{
    assert(state);
    assert(filename);
    *filename = state->filename.c_str();
    return ELEMENT_OK;
}

element_result element_tokeniser_get_input(const element_tokeniser_ctx* state, const char** input)
{
    assert(state);
    assert(input);
    *input = state->input.c_str();
    return ELEMENT_OK;
}

element_result element_tokeniser_get_token_count(const element_tokeniser_ctx* state, size_t* count)
{
    assert(state);
    assert(count);
    *count = state->tokens.size();
    return ELEMENT_OK;
}

element_result element_tokeniser_get_token(const element_tokeniser_ctx* state, const size_t index, const element_token** token)
{
    assert(state);
    assert(token);
    if (index >= state->tokens.size()) {

        auto msg = fmt::format("Internal Error - Tried to access token at index {} but there are only {} tokens",
            index, state->tokens.size());
        
        state->log(ELEMENT_ERROR_ACCESSED_TOKEN_PAST_END, msg);
        return ELEMENT_ERROR_ACCESSED_TOKEN_PAST_END;
    }
    *token = &state->tokens[index];
    return ELEMENT_OK;
}

void element_tokeniser_set_log_callback(element_tokeniser_ctx* state, void (*log_callback)(const element_log_message*))
{
    assert(log_callback);
    state->log_callback = log_callback;
}

static void reset_token(element_tokeniser_ctx* state)
{
    if (state->cur_token.type != ELEMENT_TOK_NONE || state->cur_token.post_len > 0) {
        // save current token
        state->tokens.push_back(state->cur_token);
        state->cur_token.pre_pos = -1;
        state->cur_token.pre_len = 0;
    }

    state->cur_token.type = ELEMENT_TOK_NONE;
    state->cur_token.tok_pos = -1;
    state->cur_token.tok_len = 0;
    state->cur_token.post_pos = -1;
    state->cur_token.post_len = 0;

    state->cur_token.line = -1;
    state->cur_token.column = -1;
    state->cur_token.line_start_position = -1;
}

// literal ::= [-+]? [0-9]+ ('.' [0-9]*)? ([eE] [-+]? [0-9]+)?
static element_result tokenise_number(std::string::iterator& it, const std::string::iterator& end, element_tokeniser_ctx* state)
{
    //TODO: Go through this in detail, also, waaaaaaaaaaaaaaaaaaaaay too long for parsing a number
    assert(state->cur_token.type == ELEMENT_TOK_NONE);
    state->cur_token.type = ELEMENT_TOK_NUMBER;
    state->cur_token.tok_pos = state->pos;
    state->cur_token.line = state->line;
    state->cur_token.line_start_position = state->line_start_position;
    state->cur_token.column = state->col;

    const auto it_begin = it;
    uint32_t c = UTF8_PEEK_NEXT(it, end);
    if (c == '-' || c == '+') {
        UTF8_NEXT(it, end);
        c = UTF8_PEEK_NEXT(it, end);
    }
    assert(element_isdigit(c));

    while (it != end && element_isdigit(UTF8_PEEK_NEXT(it, end)))
        c = UTF8_NEXT(it, end);

    if (it == end)
        return ELEMENT_ERROR_CONSUMED_ALL_INPUT;

    c = UTF8_PEEK_NEXT(it, end);
    if (c == '.') {
        auto it_next = it;
        UTF8_NEXT(it_next, end);

        auto c_next = UTF8_PEEK_NEXT(it_next, end);
        if (element_isdigit(c_next)) {
            // number
            UTF8_NEXT(it, end);
            while (it != end && element_isdigit(UTF8_PEEK_NEXT(it, end)))
                c = UTF8_NEXT(it, end);
        } 
        else if (element_isalpha(c_next)) {
            // indexing into a literal, do nothing and let the cleanup back out
        } else {
            const auto it_rhs_start = it_next;
            UTF8_NEXT(it_next, end);
            state->log(ELEMENT_ERROR_PARSE,
                fmt::format("Found {} which was thought to be a number being indexed, "
                    "but encountered invalid character '{}' on the right hand side of '.'",
                    std::string(it_begin, it), std::string(it_rhs_start, it_next)));

            return ELEMENT_ERROR_PARSE;
        }
    }

    c = UTF8_PEEK_NEXT(it, end);
    if (c == 'e' || c == 'E') {
        auto it_prev_character = it;
        UTF8_NEXT(it, end);
        c = UTF8_PEEK_NEXT(it, end);

        if (c == '-' || c == '+') {
            it_prev_character = it;
            UTF8_NEXT(it, end);
            c = UTF8_PEEK_NEXT(it, end);
        }

        if (!element_isdigit(c)) {
            state->log(ELEMENT_ERROR_PARSE,
                fmt::format("Found {} which was thought to be a number in scientific notation, "
                    "but encountered invalid character '{}' instead of the exponent number", 
                    std::string(it_begin, it), std::string(it_prev_character, it)));
            return ELEMENT_ERROR_PARSE;
        }

        while (it != end && element_isdigit(UTF8_PEEK_NEXT(it, end)))
            c = UTF8_NEXT(it, end);
    }

    // determine length in bytes
    const size_t len = std::distance(it_begin, it);
    state->pos += (int)len;
    state->col += (int)len;
    state->cur_token.tok_len += (int)len;
    reset_token(state);
    return ELEMENT_OK;
}

static element_result tokenise_comment(std::string::iterator& it, const std::string::iterator& end, element_tokeniser_ctx* state)
{
    //TODO: Go through this in detail
    if (state->cur_token.post_pos < 0)
        state->cur_token.post_pos = state->pos;

    //consume all characters until end of file or end of line
    const auto it_before = it;
    try
    {
        while (it != end && !element_iseol(UTF8_PEEK_NEXT(it, end))) { //will throw at EOF
            UTF8_NEXT(it, end);
        }
    }
    catch (...)
    {
        //we probably hit EOF. not the nicest way of handling this issue
    }

    // calculate correct length
    const size_t len = std::distance(it_before, it);
    state->cur_token.post_len += (int)len;
    state->pos += (int)len;

    return ELEMENT_OK;
}

static inline bool isid_alpha(uint32_t c) { return element_isalpha(c) || c == '_' || (c >= 0x00F0 && c <= 0xFFFF); }
static inline bool isid_alnum(uint32_t c) { return element_isalnum(c) || (c >= 0x00F0 && c <= 0xFFFF); }

// identifier ::= '_'? [a-zA-Z\u00F0-\uFFFF] [_a-zA-Z0-9\u00F0-\uFFFF]*
static element_result tokenise_identifier(std::string::iterator& it, const std::string::iterator& end, element_tokeniser_ctx* state)
{
    //TODO: Go through this in detail
    assert(state->cur_token.type == ELEMENT_TOK_NONE);
    state->cur_token.type = ELEMENT_TOK_IDENTIFIER;
    state->cur_token.tok_pos = state->pos;
    state->cur_token.line = state->line;
    state->cur_token.line_start_position = state->line_start_position;
    state->cur_token.column = state->col;
    const auto it_begin = it;
    uint32_t c = UTF8_PEEK_NEXT(it, end);
    if (c == '_') {
        c = UTF8_NEXT(it, end);
    }

    assert(isid_alpha(c));

    while (it != end && isid_alnum(UTF8_PEEK_NEXT(it, end))) {
        c = UTF8_NEXT(it, end);
    }

    // determine length in bytes
    const size_t len = std::distance(it_begin, it);
    state->pos += (int)len;
    state->col += (int)len;
    state->cur_token.tok_len += (int)len;
    reset_token(state);
    return ELEMENT_OK;
}

static void add_token(element_tokeniser_ctx* state, element_token_type t, int n)
{
    assert(state->cur_token.type == ELEMENT_TOK_NONE);
    state->cur_token.type = t;
    state->cur_token.tok_pos = state->pos;
    state->cur_token.tok_len = n;
    state->cur_token.line = state->line;
    state->cur_token.line_start_position = state->line_start_position;
    state->cur_token.column = state->col;
    state->pos += n;
    reset_token(state);
}

element_result element_tokeniser_create(element_tokeniser_ctx** state)
{
    *state = new element_tokeniser_ctx();
    reset_token(*state);
    return ELEMENT_OK;
}

element_result element_tokeniser_delete(element_tokeniser_ctx* state)
{
    delete state;
    return ELEMENT_OK;
}

element_result element_tokeniser_clear(element_tokeniser_ctx* state)
{
    state->tokens.clear();
    state->filename.clear();
    state->input.clear();
    state->line = 1;
    state->line_start_position = 0;
    state->col = 1;
    state->pos = 0;
    reset_token(state);
    return ELEMENT_OK;
}

element_result element_tokeniser_run(element_tokeniser_ctx* state, const char* cinput, const char* cfilename)
{
    state->filename = cfilename;
    state->input = cinput;
    state->pos = 0;
    state->line = 1;
    state->col = 1;
    reset_token(state);

    try
    {
        auto it = state->input.begin();
        auto end = state->input.end();
        uint32_t c;
        while (it != end) 
        {
            c = UTF8_PEEK_NEXT(it, end);
            switch (c) 
            {
                case '.': add_token(state, ELEMENT_TOK_DOT, 1); UTF8_NEXT(it, end); break;
                case '(': add_token(state, ELEMENT_TOK_BRACKETL, 1); UTF8_NEXT(it, end); break;
                case ')': add_token(state, ELEMENT_TOK_BRACKETR, 1); UTF8_NEXT(it, end); break;
                case ';': add_token(state, ELEMENT_TOK_SEMICOLON, 1); UTF8_NEXT(it, end); break;
                case ',': add_token(state, ELEMENT_TOK_COMMA, 1); UTF8_NEXT(it, end); break;
                case ':': add_token(state, ELEMENT_TOK_COLON, 1); UTF8_NEXT(it, end); break;
                case '{': add_token(state, ELEMENT_TOK_BRACEL, 1); UTF8_NEXT(it, end); break;
                case '}': add_token(state, ELEMENT_TOK_BRACER, 1); UTF8_NEXT(it, end); break;
                case '=': add_token(state, ELEMENT_TOK_EQUALS, 1); UTF8_NEXT(it, end); break;
                case '#': tokenise_comment(it, end, state); break;
                case '_': {
                    auto next = UTF8_PEEK_NEXT(it + 1, end);
                    if (isid_alpha(next)) {
                        const auto begin_it = it;
                        auto result = tokenise_identifier(it, end, state);
                        if (result != ELEMENT_OK) {
                            state->log(result,
                                fmt::format("Failed to parse identifier '{}'",
                                    std::string(begin_it, it)));
                            return result;
                        }
                    }
                    else {
                        add_token(state, ELEMENT_TOK_UNDERSCORE, 1);
                        UTF8_NEXT(it, end);
                    }
                    break;
                }
                default: {
                    if (isid_alpha(c)) {
                        const auto begin_it = it;
                        auto result = tokenise_identifier(it, end, state);
                        if (result != ELEMENT_OK) {
                            state->log(result,
                                fmt::format("Failed to parse identifier '{}'",
                                    std::string(begin_it, it)));
                            return result;
                        }
                    }
                    else if (element_isdigit(c) || c == '-' || c == '+') {
                        const auto begin_it = it;
                        auto result = tokenise_number(it, end, state);
                        if (result != ELEMENT_OK) {
                            state->log(result,
                                fmt::format("Failed to parse number '{}'",
                                    std::string(begin_it, it)));
                            return result;
                        }
                    }
                    else if (element_iseol(c)) {
                        ++state->line;
                        state->col = 0;
                        state->pos += 1;
                        state->line_start_position = state->pos;
                        state->line_number_to_line_pos[state->line] = state->line_start_position;
                        reset_token(state);
                        UTF8_NEXT(it, end);
                    }
                    else if (element_isspace(c)) {
                        //just eat it!
                        state->pos += 1;
                        UTF8_NEXT(it, end);
                    }
                    else {
                        const auto begin_it = it;
                        UTF8_NEXT(it, end);
                        state->log(ELEMENT_ERROR_PARSE,
                            fmt::format("Reached unexpected state when encountering character '{}'",
                                std::string(begin_it, it)));
                        return ELEMENT_ERROR_PARSE;
                    }
                }
            }
        }
        return ELEMENT_OK;
    }
    catch (const std::exception& e)
    {
        state->log(ELEMENT_ERROR_EXCEPTION,
            fmt::format("Exception occured: {}", e.what()));

        return ELEMENT_ERROR_EXCEPTION;
    }
    catch (...) //potentially EOF when last source character is UTF?
    {
        state->log(ELEMENT_ERROR_EXCEPTION,
            fmt::format("Exception occured"));

        return ELEMENT_ERROR_EXCEPTION;
    }
}

#define PRINTCASE(a) case a: c = #a; break;
void element_tokeniser_print(const element_tokeniser_ctx* state)
{
    for (const auto& t : state->tokens) {
        const char* c;
        switch (t.type) {
            PRINTCASE(ELEMENT_TOK_NONE);
            PRINTCASE(ELEMENT_TOK_NUMBER);
            PRINTCASE(ELEMENT_TOK_IDENTIFIER);
            PRINTCASE(ELEMENT_TOK_UNDERSCORE);
            PRINTCASE(ELEMENT_TOK_DOT);
            PRINTCASE(ELEMENT_TOK_BRACKETL);
            PRINTCASE(ELEMENT_TOK_BRACKETR);
            PRINTCASE(ELEMENT_TOK_SEMICOLON);
            PRINTCASE(ELEMENT_TOK_COLON);
            PRINTCASE(ELEMENT_TOK_COMMA);
            PRINTCASE(ELEMENT_TOK_BRACEL);
            PRINTCASE(ELEMENT_TOK_BRACER);
            PRINTCASE(ELEMENT_TOK_EQUALS);
            default: "ELEMENT_TOK_<UNKNOWN>"; break;
        }
        std::string buf;
        if (t.tok_len > 0)
            buf = state->input.substr(t.tok_pos, t.tok_len);
        printf("%-10s  %s\n", c + strlen("ELEMENT_TOK_"), buf.c_str());
    }
}

void advance_to_end_of_line(std::string::const_iterator& it, const std::string::const_iterator& end)
{
    try
    {
        do
        {
            UTF8_NEXT(it, end);
        } while (it != end && !element_iseol(UTF8_PEEK_NEXT(it, end)));
    }
    catch (...)
    {
        //exceptions are thrown for just about any utf issue, ignore them
    }
}

std::string element_tokeniser_ctx::text_on_line(int line) const
{
    //lines start at 1, arrays at 0
    line--;

    if (line < 0 || line > line_number_to_line_pos.size())
        return "invalid line";

    const auto start_pos = line_number_to_line_pos[line];
    const auto start_it = input.begin() + start_pos;
    auto end_it = start_it;
    advance_to_end_of_line(end_it, input.end());

    return std::string(start_it, end_it);
}