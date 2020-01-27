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
    }
};


// replacements for C stdlib functions which rely on slow locale stuff
inline bool element_isalpha(char c) { return unsigned((c&(~(1<<5))) - 'A') <= 'Z' - 'A'; }
inline bool element_isdigit(char c) { return unsigned(c - '0') <= '9' - '0'; }
inline bool element_isalnum(char c) { return element_isalpha(c) || element_isdigit(c); }