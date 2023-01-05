#include "element/token.h"

//STD
#include <cassert>
#include <cstdio>

//LIBS
#include <fmt/format.h>

//SELF
#include "token_internal.hpp"

element_result element_tokeniser_to_string(const element_tokeniser_ctx* tokeniser, const element_token* token_to_mark, char* output_buffer, int output_buffer_size)
{
    if (!tokeniser)
        return ELEMENT_ERROR_API_TOKENISER_CTX_IS_NULL;

    if (!output_buffer)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    const auto str = tokens_to_string(tokeniser, token_to_mark);
    if (static_cast<int>(str.size()) > output_buffer_size)
        return ELEMENT_ERROR_API_INSUFFICIENT_BUFFER;

    sprintf(output_buffer, "%s", str.c_str());
    return ELEMENT_OK;
}

element_result element_tokeniser_get_source_name(const element_tokeniser_ctx* tokeniser, const char** source_name)
{
    assert(tokeniser);
    assert(source_name);

    if (!tokeniser)
        return ELEMENT_ERROR_API_TOKENISER_CTX_IS_NULL;

    if (!source_name)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *source_name = tokeniser->raw_source_name;
    return ELEMENT_OK;
}

element_result element_tokeniser_get_input(const element_tokeniser_ctx* tokeniser, const char** input)
{
    assert(tokeniser);
    assert(input);

    if (!tokeniser)
        return ELEMENT_ERROR_API_TOKENISER_CTX_IS_NULL;

    if (!input)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *input = tokeniser->input.c_str();
    return ELEMENT_OK;
}

element_result element_tokeniser_get_token_count(const element_tokeniser_ctx* tokeniser, size_t* count)
{
    assert(tokeniser);
    assert(count);

    if (!tokeniser)
        return ELEMENT_ERROR_API_TOKENISER_CTX_IS_NULL;

    if (!count)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *count = tokeniser->tokens.size();
    return ELEMENT_OK;
}

element_result element_tokeniser_get_token(const element_tokeniser_ctx* tokeniser, const size_t index, const element_token** token, const char* msg)
{
    assert(tokeniser);
    assert(token);

    if (!tokeniser)
        return ELEMENT_ERROR_API_TOKENISER_CTX_IS_NULL;

    if (!token)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    std::string message = msg ? msg : "";

    if (index >= tokeniser->tokens.size()) {

        if (message.empty()) {
            message = fmt::format("tried to access token at index {} but there are only {} tokens.",
                index, tokeniser->tokens.size());

            //if the token requested is only one after the end
            if (!tokeniser->tokens.empty() && index == tokeniser->tokens.size()) {
                message += "\nnote: perhaps your source code is incomplete.";
            } else {
                message += "\nnote: perhaps an internal compiler error.";
            }
        }

        *token = nullptr;
        if (tokeniser->tokens.empty())
            return tokeniser->log(ELEMENT_ERROR_ACCESSED_TOKEN_PAST_END, msg ? msg : "");

        if (tokeniser->logger) {
            const auto& last_token = tokeniser->tokens[tokeniser->tokens.size() - 1];
            element_log_message log_msg;
            const std::string line_in_source = tokeniser->text_on_line(last_token.line);
            log_msg.line_in_source = line_in_source.c_str();
            log_msg.message = message.c_str();
            log_msg.message_length = static_cast<int>(message.length());
            log_msg.filename = last_token.source_name;
            log_msg.length = last_token.tok_len;
            log_msg.line = last_token.line;
            log_msg.message_code = ELEMENT_ERROR_PARTIAL_GRAMMAR;
            log_msg.related_log_message = nullptr;
            log_msg.stage = ELEMENT_STAGE_PARSER; //todo: could be tokeniser
            log_msg.character = last_token.character;

            tokeniser->logger->log(log_msg);
        }

        return ELEMENT_ERROR_PARTIAL_GRAMMAR;
    }

    *token = &tokeniser->tokens[index];
    return ELEMENT_OK;
}

element_result element_tokeniser_set_log_callback(element_tokeniser_ctx* tokeniser, element_log_callback log_callback, void* user_data)
{
    assert(tokeniser);
    assert(log_callback);

    if (!tokeniser)
        return ELEMENT_ERROR_API_TOKENISER_CTX_IS_NULL;

    //todo: if log callback is null, use a default? or support no callback?

    tokeniser->set_log_callback(log_callback, user_data);
    return ELEMENT_OK;
}

element_result element_tokeniser_create(element_tokeniser_ctx** tokeniser)
{
    if (!tokeniser)
        return ELEMENT_ERROR_API_OUTPUT_IS_NULL;

    *tokeniser = new element_tokeniser_ctx();
    (*tokeniser)->reset_token();
    return ELEMENT_OK;
}

void element_tokeniser_delete(element_tokeniser_ctx** tokeniser)
{
    if (!tokeniser)
        return;

    delete *tokeniser;
    *tokeniser = nullptr;
}

element_result element_tokeniser_clear(element_tokeniser_ctx* tokeniser)
{
    if (!tokeniser)
        return ELEMENT_ERROR_API_TOKENISER_CTX_IS_NULL;

    tokeniser->clear();
    return ELEMENT_OK;
}

element_result element_tokeniser_run(element_tokeniser_ctx* tokeniser, const char* cinput, const char* csource_name)
{
    if (!tokeniser)
        return ELEMENT_ERROR_API_TOKENISER_CTX_IS_NULL;

    if (!cinput || !csource_name)
        return ELEMENT_ERROR_API_STRING_IS_NULL;

    return tokeniser->run(cinput, csource_name);
}
