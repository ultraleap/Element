#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

#include "element/token.h"
#include "common_internal.hpp"

struct element_tokeniser_ctx
{
    std::shared_ptr<element_log_ctx> logger = nullptr;
    std::string filename;
    std::string input;
    int pos = 0; //position in the source file
    int line = 1;
    int line_start_position = 0;
    int character = 1; //position in the line (starting from 1)
    element_token cur_token;
    std::vector<element_token> tokens;
    std::vector<int> line_number_to_line_pos {0};

    std::string text(const element_token* t) const
    { 
        return input.substr(t->tok_pos, t->tok_len);
    }

    std::string text_on_line(int line) const;

    void log(int message_code, const std::string& message) const
    {
        if (logger == nullptr)
            return;

        logger->log(*this, message_code, message);
    }

    void log(int message_code, const std::string& message, int length, element_log_message* related_message) const
    {
        if (logger == nullptr)
            return;

        logger->log(*this, message_code, message, length, related_message);
    }

    void set_log_callback(LogCallback callback)
    {
        logger = std::make_shared<element_log_ctx>();
        logger->callback = callback;
    }

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