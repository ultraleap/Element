#pragma once

#include <cassert>
#include <string>
#include <vector>

#include "element/token.h"

struct element_tokeniser_ctx
{
    using LogCallback = void (*)(const element_log_message* const);

    std::string filename;
    std::string input;
    int pos = 0; //position in the source file
    int line = 1;
    int line_start_position = 0;
    int col = 1; //position in the line (starting from 1)
    element_token cur_token;
    std::vector<element_token> tokens;
    LogCallback log_callback;

    std::string text(const element_token* t) const
    { 
        return input.substr(t->tok_pos, t->tok_len);
    }

    void log(int message_code, const std::string& message, message_stage stage = message_stage::ELEMENT_STAGE_TOKENISER) const
    {
        if (!log_callback)
            return;

        element_log_message log;
        log.message_code = message_code;
        log.message = message.c_str();
        log.line = line;
        log.column = col;
        log.length = -1;
        log.stage = stage;
        log.filename = filename.c_str();
        log.related_log_message = nullptr;

        log_callback(&log);
    }

    void log(int message_code, const std::string& message, int length, element_log_message* related_message) const
    {
        if (!log_callback)
            return;

        element_log_message log;
        log.message_code = message_code;
        log.message = message.c_str();
        log.line = line;
        log.column = col;
        log.length = length;
        log.stage = ELEMENT_STAGE_TOKENISER;
        log.filename = filename.c_str();
        log.related_log_message = related_message;

        log_callback(&log);
    }

    void log(const element_log_message& log) const
    {
        assert(log.stage == ELEMENT_STAGE_TOKENISER);
        assert(log.message);

        if (!log_callback)
            return;

        log_callback(&log);
    }

    void set_log_callback(LogCallback callback)
    {
        log_callback = callback;
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