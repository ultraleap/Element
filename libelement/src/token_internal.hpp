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