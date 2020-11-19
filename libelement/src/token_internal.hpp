#pragma once

//STD
#include <string>
#include <vector>
#include <memory>
#include <unordered_map>

//SELF
#include "element/token.h"
#include "common_internal.hpp"

struct element_tokeniser_ctx
{
public:
    element_tokeniser_ctx();

    element_result run(const char* cinput, const char* cfilename);
    void clear();
    void reset_token();
    void add_token(element_token_type t, int n);

    // literal ::= [-+]? [0-9]+ ('.' [0-9]*)? ([eE] [-+]? [0-9]+)?
    element_result tokenise_number(std::string::iterator& it, const std::string::iterator& end);
    element_result tokenise_comment(std::string::iterator& it, const std::string::iterator& end);
    // identifier ::= '_'? [a-zA-Z\u00F0-\uFFFF] [_a-zA-Z0-9\u00F0-\uFFFF]*
    element_result tokenise_identifier(std::string::iterator& it, const std::string::iterator& end);

    [[nodiscard]] std::string text(const element_token* t) const;
    [[nodiscard]] std::string text_on_line(int line) const;

    element_result log(element_result message_code, const std::string& message) const;
    element_result log(element_result message_code, const std::string& message, int length, element_log_message* related_message) const;
    element_result log(const std::string& message) const;
    void set_log_callback(LogCallback callback, void* user_data);

    element_token* get_token(std::size_t token_index, element_result& out_result);

    std::shared_ptr<element_log_ctx> logger = nullptr;
    const char* raw_source_name = nullptr;
    std::string input;
    int pos = 0; //position in the source file
    int line = 1;
    int line_start_position = 0;
    int character = 1; //position in the line (starting from 1)
    element_token cur_token;
    std::vector<element_token> tokens;
    std::vector<int> line_number_to_line_pos{ 0 };
};