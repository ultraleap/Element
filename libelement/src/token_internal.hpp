#pragma once

#include <string>
#include <vector>

#include "element/token.h"

struct element_tokeniser_ctx
{
    std::string filename;
    std::string input;
    int pos = 0;
    int line = 1;
    int col = 1;
    element_token cur_token;
    std::vector<element_token> tokens;

    std::string text(const element_token* t) const { return input.substr(t->tok_pos, t->tok_len); }

    element_tokeniser_ctx()
    {
        tokens.reserve(512);
        cur_token.type = ELEMENT_TOK_NONE;
        cur_token.post_pos = -1;
        cur_token.post_len = -1;
        cur_token.tok_pos = -1;
        cur_token.tok_len = -1;
        cur_token.pre_pos = -1;
        cur_token.pre_len = -1;
    }
};

// replacements for C stdlib functions which rely on slow locale stuff
inline bool element_isalpha(uint32_t c) { return unsigned((c&(~(1<<5))) - 'A') <= 'Z' - 'A'; }
inline bool element_isdigit(uint32_t c) { return unsigned(c - '0') <= '9' - '0'; }
inline bool element_isalnum(uint32_t c) { return element_isalpha(c) || element_isdigit(c); }
inline bool element_isspace(uint32_t c) { return c == ' ' || (c >= 0x09 && c <= 0x0D); }
inline bool element_iseol(uint32_t c) { return c == '\n'; }