#include "token_internal.hpp"

#include <cctype>
#include <cstring>
#include <cassert>
#include <cstdlib>
#include <cstdio>

#define INCREMENT_TOKEN_LEN(s) { ++((s)->pos); ++((s)->col); ++((s)->cur_token.tok_len); }

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
    if (index >= state->tokens.size())
        return ELEMENT_ERROR_INVALID_OPERATION;
    *token = &state->tokens[index];
    return ELEMENT_OK;
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
}

// literal ::= [-+]? [0-9]+ ('.' [0-9]*)? ([eE] [-+]? [0-9]+)?
static element_result tokenise_number(const std::string& input, element_tokeniser_ctx* state)
{
    assert(state->cur_token.type == ELEMENT_TOK_NONE);
    state->cur_token.type = ELEMENT_TOK_NUMBER;
    state->cur_token.tok_pos = state->pos;
    if (input[state->pos] == '-' || input[state->pos] == '+') {
        INCREMENT_TOKEN_LEN(state);
    }
    assert(isdigit(input[state->pos]));
    do {
        INCREMENT_TOKEN_LEN(state);
    } while (isdigit(input[state->pos]));
    if (input[state->pos] == '.') {
        INCREMENT_TOKEN_LEN(state);
        while (isdigit(input[state->pos])) {
            INCREMENT_TOKEN_LEN(state);
        }
    }
    if (input[state->pos] == 'e' || input[state->pos] == 'E') {
        INCREMENT_TOKEN_LEN(state);
        if (input[state->pos] == '-' || input[state->pos] == '+') {
            INCREMENT_TOKEN_LEN(state);
        }
        if (!isdigit(input[state->pos]))
            goto error;
        do {
            INCREMENT_TOKEN_LEN(state);
        } while (isdigit(input[state->pos]));
    }
    reset_token(state);
    return ELEMENT_OK;
error:
    return ELEMENT_ERROR_INVALID_ARCHIVE;
}

// identifier ::= '_'? [a-zA-Z\u00F0-\uFFFF] [_a-zA-Z0-9\u00F0-\uFFFF]*
static element_result tokenise_identifier(const std::string& input, element_tokeniser_ctx* state)
{
    assert(state->cur_token.type == ELEMENT_TOK_NONE);
    state->cur_token.type = ELEMENT_TOK_IDENTIFIER;
    state->cur_token.tok_pos = state->pos;
    // TODO: allow \u00F0-\uFFFF
    if (input[state->pos] == '_') {
        INCREMENT_TOKEN_LEN(state);
    }
    assert(isalpha(input[state->pos]));
    INCREMENT_TOKEN_LEN(state);
    while (isalnum(input[state->pos]) || input[state->pos] == '_') {
        INCREMENT_TOKEN_LEN(state);
    }

    reset_token(state);
    return ELEMENT_OK;
}

static void add_token(element_tokeniser_ctx* state, element_token_type t, int n)
{
    assert(state->cur_token.type == ELEMENT_TOK_NONE);
    state->cur_token.type = t;
    state->cur_token.tok_pos = state->pos;
    state->cur_token.tok_len = n;
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

element_result element_tokeniser_run(element_tokeniser_ctx* state, const char* cinput, const char* cfilename)
{
    state->filename = cfilename;
    state->input = cinput;
    state->pos = 0;
    state->line = 1;
    state->col = 1;
    reset_token(state);

    const std::string& input = state->input;
    char c;
    while (input[state->pos] != '\0') {
        c = input[state->pos];
        if (isspace(c) || state->cur_token.post_pos >= 0) {
            if (c == '\n') {
                ++state->line;
                state->col = 0;
                reset_token(state);
            } else {
                if (state->cur_token.tok_pos >= 0) {
                    if (state->cur_token.post_pos < 0)
                        state->cur_token.post_pos = state->pos;
                    ++state->cur_token.post_len;
                } else {
                    ++state->cur_token.pre_len;
                }
            }
            ++state->pos;
        } else if (c == '#') {
            if (state->cur_token.post_pos < 0)
                state->cur_token.post_pos = state->pos;
            ++state->cur_token.post_len;
            ++state->pos;
        } else if (c == '-' || c == '+') {
            if (state->cur_token.type == ELEMENT_TOK_NONE) {
                if (isdigit(input[state->pos+1])) {
                    ELEMENT_OK_OR_RETURN(tokenise_number(input, state));
                } else {
                    goto error;
                }
            } else {
                goto error;
            }
        } else if (c == '=') {
            if (state->cur_token.type == ELEMENT_TOK_NONE) {
                if (input[state->pos+1] == '>') {
                    add_token(state, ELEMENT_TOK_ARROW, 2);
                } else {
                    add_token(state, ELEMENT_TOK_EQUALS, 1);
                }
            } else {
                goto error;
            }
        } else if (isdigit(c)) {
            if (state->cur_token.type == ELEMENT_TOK_NONE) {
                ELEMENT_OK_OR_RETURN(tokenise_number(input, state));
            } else {
                goto error;
            }
        } else if (c == '_') {
            if (state->cur_token.type == ELEMENT_TOK_NONE) {
                if (isalpha(input[state->pos + 1])) {
                    ELEMENT_OK_OR_RETURN(tokenise_identifier(input, state));
                } else {
                    add_token(state, ELEMENT_TOK_UNDERSCORE, 1);
                }
            } else {
                goto error;
            }
        } else if (isalpha(c)) {
            if (state->cur_token.type == ELEMENT_TOK_NONE) {
                ELEMENT_OK_OR_RETURN(tokenise_identifier(input, state));
            } else {
                goto error;
            }
        } else {
            switch (input[state->pos]) {
            case '.': add_token(state, ELEMENT_TOK_DOT, 1); break;
            case '(': add_token(state, ELEMENT_TOK_BRACKETL, 1); break;
            case ')': add_token(state, ELEMENT_TOK_BRACKETR, 1); break;
            case ';': add_token(state, ELEMENT_TOK_SEMICOLON, 1); break;
            case ',': add_token(state, ELEMENT_TOK_COMMA, 1); break;
            case ':': add_token(state, ELEMENT_TOK_COLON, 1); break;
            case '{': add_token(state, ELEMENT_TOK_BRACEL, 1); break;
            case '}': add_token(state, ELEMENT_TOK_BRACER, 1); break;
            case '=': add_token(state, ELEMENT_TOK_EQUALS, 1); break;
            default: goto error;
            }
        }
    }
    return ELEMENT_OK;

error:
    return ELEMENT_ERROR_INVALID_ARCHIVE;
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
            PRINTCASE(ELEMENT_TOK_ARROW);
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